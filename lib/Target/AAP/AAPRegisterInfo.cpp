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
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "AAPGenRegisterInfo.inc"
#include "llvm/CodeGen/MachineFunction.h"

AAPRegisterInfo::AAPRegisterInfo() : AAPGenRegisterInfo(getLinkRegister()) {}

const uint16_t *
AAPRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector AAPRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  Reserved.set(getLinkRegister());
  Reserved.set(getStackPtrRegister());

  // Mark frame pointer as reserved if necessary
  if (TFI->hasFP(MF)) {
    Reserved.set(getFramePtrRegister());
  }

  // Restrict the size of the register set
  for (unsigned i = AAP::R8; i <= AAP::R63; i++) {
    Reserved.set(i);
  }

  return Reserved;
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

  int FrameIdx = MI.getOperand(i).getIndex();
  unsigned BaseReg = getFrameRegister(MF);

  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIdx);
  if (!TFI->hasFP(MF)) {
    Offset += MF.getFrameInfo()->getStackSize();
  }

  // fold imm into offset
  Offset += MI.getOperand(i + 1).getImm();

  // If the MachineInstr is an LEA, expand it to an ADD_i10 or SUB_i10 here
  if (MI.getOpcode() == AAP::LEA) {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    unsigned DstReg = MI.getOperand(0).getReg();

    assert(((Offset >= -1023) || (Offset <= 1023)) &&
           "Currently LEA immediates must be in the range [-1023, 1023]");

    if (Offset > 0) {
      BuildMI(MBB, &MI, DL, TII->get(AAP::ADD_i10), DstReg)
        .addReg(BaseReg)
        .addImm(Offset);
    }
    else if (Offset < 0) {                                                           
      BuildMI(MBB, &MI, DL, TII->get(AAP::SUB_i10), DstReg)
        .addReg(BaseReg)
        .addImm(-Offset);
    }
    else {
      BuildMI(MBB, &MI, DL, TII->get(AAP::MOV_r), DstReg).addReg(BaseReg);
    }
    MI.eraseFromParent();
  }
  else {
    MI.getOperand(i).ChangeToRegister(BaseReg, false);
    MI.getOperand(i + 1).ChangeToImmediate(Offset);
  }
}

unsigned AAPRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  return TFI->hasFP(MF) ? getFramePtrRegister() : getStackPtrRegister();
}


unsigned AAPRegisterInfo::getLinkRegister() {
  return AAP::R0;
}
unsigned AAPRegisterInfo::getStackPtrRegister() {
  return AAP::R1;
}
unsigned AAPRegisterInfo::getFramePtrRegister() {
  return AAP::R2;
}

