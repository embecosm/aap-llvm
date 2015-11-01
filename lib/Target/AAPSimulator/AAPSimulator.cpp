//===-- AAPSimulator.cpp - AAP Simulator  ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides an implementation of a simulated AAP
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "AAPSimulator.h"
#include <cstring>

#define GET_INSTRINFO_ENUM
#include "AAPGenInstrInfo.inc"

#define GET_REGINFO_ENUM
#include "AAPGenRegisterInfo.inc"

using namespace llvm;
using namespace AAPSim;

// Should the simulator call llvm_unreachable when executing unknown
#define UNKNOWN_SHOULD_UNREACHABLE 0

AAPSimulator::AAPSimulator() {
  std::string Error;
  TheTarget = TargetRegistry::lookupTarget("aap-none-none", Error);
  if (!TheTarget) {
    errs() << "aap-run: " << Error << "\n";
  }

  // Set up all MC/Target components needed by the disassembler
  MRI = TheTarget->createMCRegInfo("aap-none-none");
  if (!MRI) {
    errs() << "error: no register info\n";
    return;
  }

  AsmInfo = TheTarget->createMCAsmInfo(*MRI, "aap-none-none");
  if (!AsmInfo) {
    errs() << "error: no asminfo\n";
    return;
  }

  STI = TheTarget->createMCSubtargetInfo("aap-none-none", "", "");
  if (!STI) {
    errs() << "error: no subtarget info\n";
    return;
  }

  MII = TheTarget->createMCInstrInfo();
  if (!MII) {
    errs() << "error: no instruction info\n";
    return;
  }

  const MCObjectFileInfo *MOFI = new MCObjectFileInfo();
  MCContext Ctx(AsmInfo, MRI, MOFI);

  DisAsm = TheTarget->createMCDisassembler(*STI, Ctx);
  if (!DisAsm) {
    errs() << "error: no disassembler\n";
    return;
  }

  int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
  IP = TheTarget->createMCInstPrinter(
      Triple("aap-none-none"), AsmPrinterVariant, *AsmInfo, *MII, *MRI);
  if (!IP) {
    errs() << "error: no instruction printer\n";
    return;
  }
}

void AAPSimulator::WriteCodeSection(llvm::StringRef Bytes, uint32_t address) {
  for (size_t i = 0; i < Bytes.size(); i++) {
    State.setCodeMem(address + i, Bytes[i]);
  }
}

void AAPSimulator::WriteDataSection(llvm::StringRef Bytes, uint32_t address) {
  for (size_t i = 0; i < Bytes.size(); i++) {
    State.setDataMem(address + i, Bytes[i]);
  }
}

static int getLLVMReg(unsigned Reg) {
  switch (Reg) {
    default: llvm_unreachable("Invalid register");
#define REG(x) case AAP::R##x: return x;
    REG(0)  REG(1)  REG(2)  REG(3)  REG(4)  REG(5)  REG(6)  REG(7)
    REG(8)  REG(9)  REG(10) REG(11) REG(12) REG(13) REG(14) REG(15)
    REG(16) REG(17) REG(18) REG(19) REG(20) REG(21) REG(22) REG(23)
    REG(24) REG(25) REG(26) REG(27) REG(28) REG(29) REG(30) REG(31)
    REG(32) REG(33) REG(34) REG(35) REG(36) REG(37) REG(38) REG(39)
    REG(40) REG(41) REG(42) REG(43) REG(44) REG(45) REG(46) REG(47)
    REG(48) REG(49) REG(50) REG(51) REG(52) REG(53) REG(54) REG(55)
    REG(56) REG(57) REG(58) REG(59) REG(60) REG(61) REG(62) REG(63)
#undef REG
  }
}

SimStatus AAPSimulator::exec(MCInst &Inst, uint32_t pc_w, uint32_t &newpc_w) {
  switch (Inst.getOpcode()) {
    // Unknown instruction
    default:
#if UNKNOWN_SHOULD_UNREACHABLE
      llvm_unreachable("No simulator support for this instruction");
#else
      newpc_w = pc_w;
      return SimStatus::SIM_TRAP;
#endif
      break;

    // Move Instructions
    case AAP::MOV_r:
    case AAP::MOV_r_short: {
      int RegDst = getLLVMReg(Inst.getOperand(0).getReg());
      int RegSrc = getLLVMReg(Inst.getOperand(1).getReg());
      State.setReg(RegDst, State.getReg(RegSrc));
      break;
    }
    case AAP::MOVI_i16:
    case AAP::MOVI_i6_short: {
      int Reg = getLLVMReg(Inst.getOperand(0).getReg());
      uint16_t Val = Inst.getOperand(1).getImm() & 0xffff;
      State.setReg(Reg, Val);
      break;
    }

    // Branch and Link, Jump and Link
    case AAP::BAL:
    case AAP::BAL_short:
    case AAP::JAL:
    case AAP::JAL_short: {
      int Reg = getLLVMReg(Inst.getOperand(1).getReg());
      State.setReg(Reg, newpc_w);
      uint32_t Imm = Inst.getOperand(0).getImm() & 0xffffff;
      if (Inst.getOpcode() == AAP::BAL || Inst.getOpcode() == AAP::BAL_short)
        newpc_w = pc_w + Imm;
      else
        newpc_w = Imm;
      break;
    }
  }
  return SimStatus::SIM_OK;
}

SimStatus AAPSimulator::step() {
  MCInst Inst;
  uint64_t Size;
  uint32_t pc_w = State.getPC();
  ArrayRef<uint8_t> *Bytes = State.getCodeArray();

  if (DisAsm->getInstruction(Inst, Size, Bytes->slice(pc_w << 1), (pc_w << 1),
                             nulls(), nulls())) {
    // Instruction decoded, execute it and write back our PC
    dbgs() << format("%06" PRIx64 ":", pc_w);
    IP->printInst(&Inst, dbgs(), "", *STI);
    dbgs() << "\n";

    uint32_t newpc_w = pc_w + (Size >> 1);
    SimStatus status;
    status = exec(Inst, pc_w, newpc_w);
    State.setPC(newpc_w);

    return status;
  }
  else {
    return SimStatus::SIM_INVALID_INSN;
  }
}