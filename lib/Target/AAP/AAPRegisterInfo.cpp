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
#include "AAPFrameLowering.h"
#include "AAPSubtarget.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "AAPGenRegisterInfo.inc"

AAPRegisterInfo::AAPRegisterInfo() : AAPGenRegisterInfo(getLinkRegister()) {}

const uint32_t *
AAPRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID CC) const {
  return CSR_RegMask;
}

const MCPhysReg *
AAPRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector AAPRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  Reserved.set(getLinkRegister());
  Reserved.set(getStackPtrRegister());
  if (TFI->hasFP(MF))
    Reserved.set(getFramePtrRegister());

  return Reserved;
}

void AAPRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MBBI,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *MBBI;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const AAPInstrInfo *TII = MF.getSubtarget<AAPSubtarget>().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  int FrameIdx = MI.getOperand(FIOperandNum).getIndex();
  unsigned BaseReg;
  int Offset =
      getFrameLowering(MF)->getFrameIndexReference(MF, FrameIdx, BaseReg) +
      MI.getOperand(FIOperandNum + 1).getImm();

  // If the MachineInstr is an LEA, expand it to a register adjustment here.
  if (MI.getOpcode() == AAP::LEA) {
    unsigned DestReg = MI.getOperand(0).getReg();
    adjustReg(MBB, MBBI, DL, DestReg, BaseReg, Offset, MachineInstr::NoFlags);
    MI.eraseFromParent();
    return;
  }

  if (!(isInt<16>(Offset) || isUInt<16>(Offset)))
    llvm_unreachable("eliminateFrameIndex cannot handle offsets > 16 bits");

  bool BaseRegIsKill = false;

  if (!isInt<10>(Offset)) {
    unsigned ScratchReg = MRI.createVirtualRegister(&AAP::GR64RegClass);
    BuildMI(MBB, MBBI, DL, TII->get(AAP::MOVI_i16), ScratchReg).addImm(Offset);
    BuildMI(MBB, MBBI, DL, TII->get(AAP::ADD_r), ScratchReg)
        .addReg(BaseReg)
        .addReg(ScratchReg, RegState::Kill);
    Offset = 0;
    BaseReg = ScratchReg;
    BaseRegIsKill = true;
  }

  MI.getOperand(FIOperandNum)
      .ChangeToRegister(BaseReg, false, false, BaseRegIsKill);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

unsigned AAPRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  return TFI->hasFP(MF) ? getFramePtrRegister() : getStackPtrRegister();
}

void AAPRegisterInfo::adjustReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MBBI,
                                const DebugLoc &DL, unsigned DestReg,
                                unsigned SrcReg, int64_t Val,
                                MachineInstr::MIFlag Flag) {
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

unsigned AAPRegisterInfo::getLinkRegister() { return AAP::R0; }
unsigned AAPRegisterInfo::getStackPtrRegister() { return AAP::R1; }
unsigned AAPRegisterInfo::getFramePtrRegister() { return AAP::R8; }
