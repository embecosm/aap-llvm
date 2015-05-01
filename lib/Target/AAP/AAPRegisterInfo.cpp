//===-- AAPRegisterInfo.cpp - AAP Register Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "AAPRegisterInfo.h"
#include "AAP.h"
#include "AAPSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "AAPGenRegisterInfo.inc"
#include "llvm/CodeGen/MachineFunction.h"

AAPRegisterInfo::AAPRegisterInfo() : AAPGenRegisterInfo(AAP::R1) {}

const uint16_t *
AAPRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const TargetFrameLowering *TFI = MF->getSubtarget().getFrameLowering();

  return CSR_SaveList;
}

BitVector AAPRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  Reserved.set(AAP::R0); // link register
  Reserved.set(AAP::R1); // stack pointer
  if (TFI->hasFP(MF)) {
    Reserved.set(AAP::R2); // frame pointer
  }

  return Reserved;

  // TODO: Reserve unused registers here?
}

bool AAPRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return false;
}

void AAPRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MBBI,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *MBBI;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  DebugLoc DL = MI.getDebugLoc();

  unsigned i = 0;
  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() &&
           "Instr does not have a Frame Index operand!");
  }

  assert(!TFI->hasFP(MF) && "Frame pointer not supported!");

  int FrameIdx = MI.getOperand(i).getIndex();
  unsigned BaseReg = getFrameRegister(MF);

  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIdx);
  Offset += MF.getFrameInfo()->getStackSize();

  // fold imm into offset
  Offset += MI.getOperand(i + 1).getImm();

  MI.getOperand(i).ChangeToRegister(BaseReg, false);
  MI.getOperand(i + 1).ChangeToImmediate(Offset);
}

unsigned AAPRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  return TFI->hasFP(MF) ? AAP::R2 : AAP::R1;
}
