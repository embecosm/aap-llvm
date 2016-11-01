//===-- AAPTargetInfo.cpp - AAP Target Implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace llvm {
Target &getTheAAPTarget() {
  static Target TheAAPTarget;
  return TheAAPTarget;
}
}

extern "C" void LLVMInitializeAAPTargetInfo() {
  RegisterTarget<Triple::aap> X(getTheAAPTarget(), "aap", "AAP [experimental]");
}
