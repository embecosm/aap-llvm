//===-- AAPFrameLowering.cpp - Frame info for AAP Target ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "AAPFrameLowering.h"
#include "AAPInstrInfo.h"
#include "AAPMachineFunctionInfo.h"
#include "AAPSubtarget.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// AAPFrameLowering:
//===----------------------------------------------------------------------===//

AAPFrameLowering::AAPFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 2, 0, 2) {}

bool AAPFrameLowering::hasFP(const MachineFunction &MF) const {
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MF.getFrameInfo().hasVarSizedObjects();
}

void AAPFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFrameInfo = MF.getFrameInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();
  const AAPInstrInfo &TII =
      *static_cast<const AAPInstrInfo *>(MF.getSubtarget().getInstrInfo());

  auto MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo
  const uint64_t StackSize = MFrameInfo.getStackSize();

  assert(!hasFP(MF) && "Frame pointer unsupported!");

  uint64_t NumBytes = StackSize - MFuncInfo->getCalleeSavedFrameSize();
  const unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  // Adjust the stack pointer if there is a stack to allocate
  if (NumBytes) {
    const uint64_t Addend = NumBytes % 1023;
    const uint64_t NumChunks = NumBytes / 1023;

    for (uint64_t i = 0; i < NumChunks; ++i) {
      BuildMI(MBB, MBBI, DL, TII.get(AAP::SUBI_i10), SP)
          .addReg(SP)
          .addImm(1023);
    }
    if (Addend) {
      BuildMI(MBB, MBBI, DL, TII.get(AAP::SUBI_i10), SP)
          .addReg(SP)
          .addImm(Addend);
    }
  }
}

void AAPFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();
  const AAPInstrInfo &TII =
      *static_cast<const AAPInstrInfo *>(MF.getSubtarget().getInstrInfo());

  auto MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  unsigned RetOpcode = MBBI->getOpcode();
  assert((RetOpcode == AAP::JMP) &&
         "Epilogue can only be inserted in returning blocks");

  // Number of bytes to dealloc from FrameInfo
  const uint64_t StackSize = MFrameInfo.getStackSize();
  uint64_t NumBytes = StackSize - MFuncInfo->getCalleeSavedFrameSize();

  const unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  assert(!hasFP(MF) && "Frame pointer unsupported!");

  if (NumBytes) {
    // otherwise adjust by adding back the frame size
    const uint64_t Addend = NumBytes % 1023;
    const uint64_t NumChunks = NumBytes / 1023;

    for (uint64_t i = 0; i < NumChunks; ++i) {
      BuildMI(MBB, MBBI, DL, TII.get(AAP::ADDI_i10), SP)
          .addReg(SP)
          .addImm(1023);
    }
    if (Addend) {
      BuildMI(MBB, MBBI, DL, TII.get(AAP::ADDI_i10), SP)
          .addReg(SP)
          .addImm(Addend);
    }
  }
}

// This function eliminates ADJCALLSTACKDOWN,
// ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator AAPFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // FIXME: Implement frame pointer support
  assert(!hasFP(MF) && "Frame pointer unsupported!");
  return MBB.erase(I);
}

void AAPFrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {}
