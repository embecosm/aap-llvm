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

#include "AAPSimState.h"

namespace AAPSim {

/// AAPSimulator - AAP Simulator
class AAPSimulator {
  AAPSimState State;

public:
  AAPSimulator();

  AAPSimState &getState() { return State; }
  void WriteCodeSection(llvm::StringRef Bytes, uint32_t address);
  void setPC(uint32_t pc_w) { State.setPC(pc_w); }
};

} // End AAPSim namespace

#endif
