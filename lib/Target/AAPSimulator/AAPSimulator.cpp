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

#include "llvm/Support/TargetRegistry.h"
#include "AAPSimulator.h"
#include <cstring>

using namespace llvm;
using namespace AAPSim;

AAPSimulator::AAPSimulator() {
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget("aap-none-none", Error);
  if (!TheTarget) {
    errs() << "aap-run: " << Error << "\n";
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