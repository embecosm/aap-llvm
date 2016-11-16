//===------- AAPShortInstrPeephole.cpp - Pick shorter instructions --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Simple pass to replace long instructions with shorter equivalents
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "short-instr-peephole"

#include "AAPRegisterInfo.h"
#include "AAPTargetMachine.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {
struct ShortInstrPeephole : public MachineFunctionPass {
  /// Target machine description used to query for register names, data
  /// layout and similar.
  TargetMachine &TM;
  const MCInstrInfo &MII;

  static char ID;
  ShortInstrPeephole(TargetMachine &TM)
      : MachineFunctionPass(ID), TM(TM), MII(*TM.getMCInstrInfo()) {}

  StringRef getPassName() const override {
    return "AAP Short Instruction Peephole";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool runOnInstruction(MachineInstr &MI) const;

  bool updateMOV_r(MachineInstr &MI) const;
  bool updateMOVI_i16(MachineInstr &MI) const;
  bool updateNOP(MachineInstr &MI) const;
  bool updateALU_r(MachineInstr &MI) const;
  bool updateARITH_i10(MachineInstr &MI) const;
  bool updateSHIFT_i6(MachineInstr &MI) const;
  bool updateLD(MachineInstr &MI) const;
  bool updateST(MachineInstr &MI) const;
  bool updateBRA(MachineInstr &MI) const;
  bool updateBAL(MachineInstr &MI) const;
  bool updateJMP(MachineInstr &MI) const;
  bool updateJAL(MachineInstr &MI) const;
};

char ShortInstrPeephole::ID = 0;
} // end of anonymous namespace

FunctionPass *llvm::createAAPShortInstrPeepholePass(AAPTargetMachine &TM) {
  return new ShortInstrPeephole(TM);
}

bool ShortInstrPeephole::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      Changed |= runOnInstruction(MI);
    }
  }
  return Changed;
}

bool ShortInstrPeephole::runOnInstruction(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case AAP::MOV_r:
    return updateMOV_r(MI);
  case AAP::MOVI_i16:
    return updateMOVI_i16(MI);
  case AAP::NOP:
    return updateNOP(MI);

  case AAP::ADD_r:
  case AAP::AND_r:
  case AAP::OR_r:
  case AAP::XOR_r:
  case AAP::SUB_r:
  case AAP::ASR_r:
  case AAP::LSL_r:
  case AAP::LSR_r:
    return updateALU_r(MI);

  case AAP::ADDI_i10:
  case AAP::SUBI_i10:
    return updateARITH_i10(MI);

  case AAP::ASRI_i6:
  case AAP::LSLI_i6:
  case AAP::LSRI_i6:
    return updateSHIFT_i6(MI);

  case AAP::LDB:
  case AAP::LDW:
  case AAP::LDB_postinc:
  case AAP::LDW_postinc:
  case AAP::LDB_predec:
  case AAP::LDW_predec:
    return updateLD(MI);

  case AAP::STB:
  case AAP::STW:
  case AAP::STB_postinc:
  case AAP::STW_postinc:
  case AAP::STB_predec:
  case AAP::STW_predec:
    return updateST(MI);

  case AAP::BRA:
    return updateBRA(MI);
  case AAP::BAL:
    return updateBAL(MI);
  case AAP::JAL:
    return updateJAL(MI);
  case AAP::JMP:
    return updateJMP(MI);

  default:
    return false;
  }
}

bool ShortInstrPeephole::updateMOV_r(MachineInstr &MI) const {
  const auto &DstReg = MI.getOperand(0).getReg();
  const auto &SrcReg = MI.getOperand(1).getReg();

  if (AAP::GR8RegClass.contains(DstReg, SrcReg)) {
    MI.setDesc(MII.get(AAP::MOV_r_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateMOVI_i16(MachineInstr &MI) const {
  unsigned DstReg = MI.getOperand(0).getReg();
  const auto &Imm = MI.getOperand(1);

  if (AAP::GR8RegClass.contains(DstReg) && Imm.isImm() &&
      AAP::isImm6(Imm.getImm())) {
    MI.setDesc(MII.get(AAP::MOVI_i6_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateNOP(MachineInstr &MI) const {
  unsigned SrcReg = MI.getOperand(0).getReg();
  const auto &Imm = MI.getOperand(1);

  if (AAP::GR8RegClass.contains(SrcReg) && Imm.isImm() &&
      AAP::isImm6(Imm.getImm())) {
    MI.setDesc(MII.get(AAP::NOP_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateALU_r(MachineInstr &MI) const {
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned Op1Reg = MI.getOperand(1).getReg();
  unsigned Op2Reg = MI.getOperand(2).getReg();

  if (AAP::GR8RegClass.contains(DstReg) && AAP::GR8RegClass.contains(Op1Reg) &&
      AAP::GR8RegClass.contains(Op2Reg)) {
    unsigned Opcode = MI.getOpcode();
    switch (Opcode) {
    case AAP::ADD_r:
      Opcode = AAP::ADD_r_short;
      break;
    case AAP::AND_r:
      Opcode = AAP::AND_r_short;
      break;
    case AAP::OR_r:
      Opcode = AAP::OR_r_short;
      break;
    case AAP::XOR_r:
      Opcode = AAP::XOR_r_short;
      break;
    case AAP::SUB_r:
      Opcode = AAP::SUB_r_short;
      break;
    case AAP::ASR_r:
      Opcode = AAP::ASR_r_short;
      break;
    case AAP::LSL_r:
      Opcode = AAP::LSL_r_short;
      break;
    case AAP::LSR_r:
      Opcode = AAP::LSR_r_short;
      break;
    default:
      llvm_unreachable("Unknown opcode");
    }
    MI.setDesc(MII.get(Opcode));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateARITH_i10(MachineInstr &MI) const {
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(1).getReg();
  const auto &Imm = MI.getOperand(2);

  if (AAP::GR8RegClass.contains(DstReg, SrcReg) && Imm.isImm() &&
      AAP::isImm3(Imm.getImm())) {
    unsigned Opcode = MI.getOpcode();
    switch (Opcode) {
    case AAP::ADDI_i10:
      Opcode = AAP::ADDI_i3_short;
      break;
    case AAP::SUBI_i10:
      Opcode = AAP::SUBI_i3_short;
      break;
    }
    MI.setDesc(MII.get(Opcode));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateSHIFT_i6(MachineInstr &MI) const {
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(1).getReg();
  const auto &Imm = MI.getOperand(2);

  if (AAP::GR8RegClass.contains(DstReg, SrcReg) && Imm.isImm() &&
      AAP::isShiftImm3(Imm.getImm())) {
    unsigned Opcode = MI.getOpcode();
    switch (Opcode) {
    case AAP::ASRI_i6:
      Opcode = AAP::ASRI_i3_short;
      break;
    case AAP::LSLI_i6:
      Opcode = AAP::LSLI_i3_short;
      break;
    case AAP::LSRI_i6:
      Opcode = AAP::LSRI_i3_short;
      break;
    }
    MI.setDesc(MII.get(Opcode));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateLD(MachineInstr &MI) const {
  unsigned DstReg = MI.getOperand(0).getReg();
  const auto &Base = MI.getOperand(1);
  const auto &Off = MI.getOperand(2);

  if (AAP::GR8RegClass.contains(DstReg) && Base.isReg() &&
      AAP::GR8RegClass.contains(Base.getReg()) && Off.isImm() &&
      AAP::isOff3(Off.getImm())) {
    unsigned Opcode = MI.getOpcode();
    switch (Opcode) {
    case AAP::LDB:
      Opcode = AAP::LDB_short;
      break;
    case AAP::LDW:
      Opcode = AAP::LDW_short;
      break;
    case AAP::LDB_postinc:
      Opcode = AAP::LDB_postinc_short;
      break;
    case AAP::LDW_postinc:
      Opcode = AAP::LDW_postinc_short;
      break;
    case AAP::LDB_predec:
      Opcode = AAP::LDB_predec_short;
      break;
    case AAP::LDW_predec:
      Opcode = AAP::LDW_predec_short;
      break;
    }
    MI.setDesc(MII.get(Opcode));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateST(MachineInstr &MI) const {
  const auto &Base = MI.getOperand(0);
  const auto &Off = MI.getOperand(1);
  unsigned SrcReg = MI.getOperand(2).getReg();

  if (Base.isReg() && AAP::GR8RegClass.contains(Base.getReg()) && Off.isImm() &&
      AAP::isOff3(Off.getImm()) && AAP::GR8RegClass.contains(SrcReg)) {
    unsigned Opcode = MI.getOpcode();
    switch (Opcode) {
    case AAP::STB:
      Opcode = AAP::STB_short;
      break;
    case AAP::STW:
      Opcode = AAP::STW_short;
      break;
    case AAP::STB_postinc:
      Opcode = AAP::STB_postinc_short;
      break;
    case AAP::STW_postinc:
      Opcode = AAP::STW_postinc_short;
      break;
    case AAP::STB_predec:
      Opcode = AAP::STB_predec_short;
      break;
    case AAP::STW_predec:
      Opcode = AAP::STW_predec_short;
      break;
    }
    MI.setDesc(MII.get(Opcode));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateBRA(MachineInstr &MI) const {
  const auto &MO = MI.getOperand(0);
  if (MO.isImm() && AAP::isOff9(MO.getImm())) {
    MI.setDesc(MII.get(AAP::BRA_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateBAL(MachineInstr &MI) const {
  const auto &MO = MI.getOperand(0);
  unsigned LinkReg = MI.getOperand(1).getReg();

  if (MO.isImm() && AAP::isOff6(MO.getImm()) &&
      AAP::GR8RegClass.contains(LinkReg)) {
    MI.setDesc(MII.get(AAP::BAL_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateJMP(MachineInstr &MI) const {
  unsigned LinkReg = MI.getOperand(0).getReg();
  if (AAP::GR8RegClass.contains(LinkReg)) {
    MI.setDesc(MII.get(AAP::JMP_short));
    return true;
  }
  return false;
}

bool ShortInstrPeephole::updateJAL(MachineInstr &MI) const {
  unsigned TargetReg = MI.getOperand(0).getReg();
  unsigned LinkReg = MI.getOperand(1).getReg();

  if (AAP::GR8RegClass.contains(TargetReg, LinkReg)) {
    MI.setDesc(MII.get(AAP::JAL_short));
    return true;
  }
  return false;
}
