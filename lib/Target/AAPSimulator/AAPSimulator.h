//===-- AAPSimulator.h - AAP Simulator  -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of a simulated AAP
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAPSIMULATOR_AAPSIMULATOR_H
#define LLVM_LIB_TARGET_AAPSIMULATOR_AAPSIMULATOR_H

#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "AAPSimState.h"

namespace AAPSim {

/// AAPSimulator - AAP Simulator
class AAPSimulator {
  AAPSimState State;

  // Target/MCInfo
  const llvm::Target *TheTarget;
  const llvm::MCRegisterInfo *MRI;
  const llvm::MCAsmInfo *AsmInfo;
  const llvm::MCSubtargetInfo *STI;
  const llvm::MCInstrInfo *MII;
  llvm::MCDisassembler *DisAsm;
  llvm::MCInstPrinter *IP;

public:
  AAPSimulator();

  AAPSimState &getState() { return State; }

  /// Functions for writing bulk to the code/data memories
  void WriteCodeSection(llvm::StringRef Bytes, uint32_t address);
  void WriteDataSection(llvm::StringRef Bytes, uint32_t address);

  /// Set Program Counter
  void setPC(uint32_t pc_w) { State.setPC(pc_w); }

  /// Execute an instruction
  SimStatus exec(llvm::MCInst &Inst, uint32_t pc_w, uint32_t &newpc_w);

  /// Step the processor
  SimStatus step();

  /// Trace control
  bool getTracing() const { return State.getTracing(); }
  void setTracing(bool enabled) { State.setTracing(enabled); }
};

} // End AAPSim namespace

#endif
