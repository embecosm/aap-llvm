//===-- AAPInstrInfo.cpp - AAP Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "AAPInstrInfo.h"
#include "AAPTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "AAPGenInstrInfo.inc"

AAPInstrInfo::AAPInstrInfo()
    : AAPGenInstrInfo(AAP::ADJCALLSTACKDOWN, AAP::ADJCALLSTACKUP) {}


unsigned AAPInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  switch (MI.getOpcode()) {
  default:
    return 0;
  case AAP::STB:
  case AAP::STW:
  case AAP::STB_postinc:
  case AAP::STW_postinc:
  case AAP::STB_predec:
  case AAP::STW_predec:
    break;
  }

  if (MI.getOperand(0).isFI() && MI.getOperand(1).isImm() &&
      MI.getOperand(1).getImm() == 0) {
    FrameIndex = MI.getOperand(0).getIndex();
    return MI.getOperand(2).getReg();
  }

  return 0;
}

unsigned AAPInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  default:
    return 0;
  case AAP::LDB:
  case AAP::LDW:
  case AAP::LDB_postinc:
  case AAP::LDW_postinc:
  case AAP::LDB_predec:
  case AAP::LDW_predec:
    break;
  }

  if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
      MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }

  return 0;
}

void AAPInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, unsigned DestReg,
                               unsigned SrcReg, bool KillSrc) const {
  assert(AAP::GR64RegClass.contains(DestReg) &&
         AAP::GR64RegClass.contains(DestReg) &&
         "Impossible register-register copy");

  // FIXME: If possible with short insn, build that instead
  BuildMI(MBB, I, DL, get(AAP::MOV_r), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

void AAPInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       unsigned SrcReg, bool isKill,
                                       int FrameIdx,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIdx),
      MachineMemOperand::MOStore, MFrameInfo.getObjectSize(FrameIdx),
      MFrameInfo.getObjectAlignment(FrameIdx));

  assert((RC == &AAP::GR8RegClass || RC == &AAP::GR64RegClass) &&
         "Unknown register class to store to stack slot");

  BuildMI(MBB, MI, DL, get(AAP::STW))
      .addFrameIndex(FrameIdx)
      .addImm(0)
      .addReg(SrcReg, getKillRegState(isKill))
      .addMemOperand(MMO);
}

void AAPInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        unsigned DstReg, int FrameIdx,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIdx),
      MachineMemOperand::MOLoad, MFrameInfo.getObjectSize(FrameIdx),
      MFrameInfo.getObjectAlignment(FrameIdx));

  assert((RC == &AAP::GR8RegClass || RC == &AAP::GR64RegClass) &&
         "Unknown register class to store to stack slot");

  BuildMI(MBB, MI, DL, get(AAP::LDW), DstReg)
      .addFrameIndex(FrameIdx)
      .addImm(0)
      .addMemOperand(MMO);
}

AAPCC::CondCode AAPInstrInfo::getCondFromBranchOpcode(unsigned Opcode) {
  switch (Opcode) {
  default:
    return AAPCC::COND_INVALID;
  case AAP::BEQ_:
    return AAPCC::COND_EQ;
  case AAP::BNE_:
    return AAPCC::COND_NE;
  case AAP::BLTS_:
    return AAPCC::COND_LTS;
  case AAP::BLES_:
    return AAPCC::COND_LES;
  case AAP::BLTU_:
    return AAPCC::COND_LTU;
  case AAP::BLEU_:
    return AAPCC::COND_LEU;
  }
}

unsigned AAPInstrInfo::getBranchOpcodeFromCond(AAPCC::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Invalid condition code");
  case AAPCC::COND_EQ:
    return AAP::BEQ_;
  case AAPCC::COND_NE:
    return AAP::BNE_;
  case AAPCC::COND_LTS:
    return AAP::BLTS_;
  case AAPCC::COND_LES:
    return AAP::BLES_;
  case AAPCC::COND_LTU:
    return AAP::BLTU_;
  case AAPCC::COND_LEU:
    return AAP::BLEU_;
  }
}

AAPCC::CondCode AAPInstrInfo::reverseCondCode(AAPCC::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Invalid condition code");
  case AAPCC::COND_EQ:
    return AAPCC::COND_NE;
  case AAPCC::COND_NE:
    return AAPCC::COND_EQ;
  case AAPCC::COND_LES:
  case AAPCC::COND_LEU:
  case AAPCC::COND_LTS:
  case AAPCC::COND_LTU:
    return AAPCC::COND_INVALID;
  }
}
