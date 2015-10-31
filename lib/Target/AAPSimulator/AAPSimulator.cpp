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
#include "AAPInstrInfo.h"
#include "AAPSimulator.h"
#include <cstring>

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