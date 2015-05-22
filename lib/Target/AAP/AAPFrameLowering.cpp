//===-- AAPFrameLowering.cpp - Frame info for AAP Target ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains AAP frame information that doesn't fit anywhere else
// cleanly...
//
//===----------------------------------------------------------------------===//

#include "AAPFrameLowering.h"
#include "AAP.h"
#include "AAPInstrInfo.h"
#include "AAPMachineFunctionInfo.h"
#include "AAPSubtarget.h"
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
         MF.getFrameInfo()->hasVarSizedObjects();
}

void AAPFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front(); // Prolog in entry BB

  MachineFrameInfo *MFrameInfo = MF.getFrameInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();
  const AAPInstrInfo &TII =
      *static_cast<const AAPInstrInfo *>(MF.getSubtarget().getInstrInfo());

  auto MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo
  uint64_t StackSize = MFrameInfo->getStackSize();

  if (hasFP(MF)) {
    llvm_unreachable("Cannot handle the presence of a frame pointer!");
  } else {
    uint64_t NumBytes = StackSize - MFuncInfo->getCalleeSavedFrameSize();

    if (NumBytes) {
      // Adjust stack pointer
      const unsigned SP = AAPRegisterInfo::getStackPtrRegister();
      BuildMI(MBB, MBBI, DL, TII.get(AAP::SUB_i10), SP)
          .addReg(SP)
          .addImm(NumBytes);
    }
  }
}

void AAPFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo *MFrameInfo = MF.getFrameInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();
  const AAPInstrInfo &TII =
      *static_cast<const AAPInstrInfo *>(MF.getSubtarget().getInstrInfo());

  auto MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  unsigned RetOpcode = MBBI->getOpcode();
  assert(RetOpcode == AAP::JMP &&
         "Epilogue can only be inserted in returning blocks");

  // Number of bytes to dealloc from FrameInfo
  uint64_t StackSize = MFrameInfo->getStackSize();

  if (hasFP(MF)) {
    llvm_unreachable("Cannot handle the presence of a frame pointer!");
  } else {
    uint64_t NumBytes = StackSize - MFuncInfo->getCalleeSavedFrameSize();

    const unsigned SP = AAPRegisterInfo::getStackPtrRegister();
    if (NumBytes) {
      // Adjust stack pointer back
      BuildMI(MBB, MBBI, DL, TII.get(AAP::ADD_i10), SP)
          .addReg(SP)
          .addImm(NumBytes);
    }
  }
}

bool AAPFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    const std::vector<CalleeSavedInfo> &CSI,
    const TargetRegisterInfo *TRI) const {
  if (CSI.empty()) {
    return false;
  }

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  // Each spilled register is 2 bytes less adjustment to SP
  MFuncInfo->setCalleeSavedFrameSize(CSI.size() * 2);

  for (unsigned i = CSI.size(); i != 0; --i) {
    unsigned Reg = CSI[i - 1].getReg();

    // Add Callee-saved register as live-in. It's killed by the spill
    const unsigned SP = AAPRegisterInfo::getStackPtrRegister();
    MBB.addLiveIn(Reg);
    BuildMI(MBB, MI, DL, TII.get(AAP::STW_predec), SP)
        .addReg(SP)
        .addImm(0)
        .addReg(Reg, RegState::Kill);
  }
  return true;
}

bool AAPFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    const std::vector<CalleeSavedInfo> &CSI,
    const TargetRegisterInfo *TRI) const {
  if (CSI.empty()) {
    return false;
  }

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  for (unsigned i = 0; i != CSI.size(); ++i) {
    unsigned Reg = CSI[i].getReg();

    const unsigned SP = AAPRegisterInfo::getStackPtrRegister();
    BuildMI(MBB, MI, DL, TII.get(AAP::LDW_postinc), Reg)
        .addReg(SP, RegState::Define)
        .addReg(SP)
        .addImm(0);
  }
  return true;
}

// This function eliminates ADJCALLSTACKDOWN,
// ADJCALLSTACKUP pseudo instructions
void AAPFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // FIXME: Assume that we don't have frame pointer
  MBB.erase(I);
}

void AAPFrameLowering::processFunctionBeforeCalleeSavedScan(
    MachineFunction &MF, RegScavenger *RS) const {}

void AAPFrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {}
