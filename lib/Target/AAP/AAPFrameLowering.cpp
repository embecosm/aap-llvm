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
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

static void adjustReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                      const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
                      int64_t Val, MachineInstr::MIFlag Flag) {
  if (Val == 0 && DestReg == SrcReg)
    return;

  bool isSub = Val < 0;
  Val = isSub ? -Val : Val;

  if (!isUInt<16>(Val))
    llvm_unreachable("adjustReg cannot handle adjustments > 16 bits");

  const TargetInstrInfo &TII = *MBB.getParent()->getSubtarget().getInstrInfo();

  if (isUInt<10>(Val)) {
    unsigned Opcode = isSub ? AAP::SUBI_i10 : AAP::ADDI_i10;
    BuildMI(MBB, MBBI, DL, TII.get(Opcode), DestReg)
        .addReg(SrcReg)
        .addImm(Val)
        .setMIFlag(Flag);
    return;
  }

  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  unsigned ScratchReg = MRI.createVirtualRegister(&AAP::GR64RegClass);
  BuildMI(MBB, MBBI, DL, TII.get(AAP::MOVI_i16), ScratchReg)
      .addImm(Val)
      .setMIFlag(Flag);

  unsigned Opcode = isSub ? AAP::SUB_r : AAP::ADD_r;
  BuildMI(MBB, MBBI, DL, TII.get(Opcode), DestReg)
      .addReg(SrcReg)
      .addReg(ScratchReg, RegState::Kill)
      .setMIFlag(Flag);
}

bool AAPFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MF.getSubtarget().getRegisterInfo()->needsStackRealignment(MF) ||
         MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator AAPFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  // TODO: Support frame pointer.
  assert(!hasFP(MF) && "Frame pointer unsupported");
  return MBB.erase(MI);
}

void AAPFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  // Get the number of bytes to allocate from the FrameInfo.
  const uint64_t StackSize = MFrameInfo.getStackSize();

  // Adjust the stack pointer down by the number of bytes needed.
  adjustReg(MBB, MBBI, DL, SP, SP, -StackSize, MachineInstr::FrameSetup);

  // TODO: Support frame pointer adjustment.
  assert(!hasFP(MF) && "Frame pointer unsupported");
}

void AAPFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  assert((MBBI->getDesc().isReturn()) &&
         "Epilogue can only be inserted in returning blocks");
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  // Get the number of bytes to deallocate from the FrameInfo
  const uint64_t StackSize = MFrameInfo.getStackSize();

  // TODO: Support frame pointer adjustment.
  assert(!hasFP(MF) && "Frame pointer unsupported");

  // Adjust the stack pointer up by the number of bytes needed.
  adjustReg(MBB, MBBI, DL, SP, SP, StackSize, MachineInstr::FrameDestroy);
}
