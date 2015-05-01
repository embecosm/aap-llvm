//===-- AAPTargetMachine.cpp - Define TargetMachine for AAP ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about AAP target spec.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "AAPTargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeAAPTarget() {
  // Register the target
  RegisterTargetMachine<AAPTargetMachine> X(TheAAPTarget);
}

AAPTargetMachine::AAPTargetMachine(const Target &T, StringRef TT, StringRef CPU,
                                   StringRef FS, const TargetOptions &Options,
                                   Reloc::Model RM, CodeModel::Model CM,
                                   CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, "e-m:e-p:16:16-n16", TT, CPU, FS, Options, RM, CM,
                        OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

AAPTargetMachine::~AAPTargetMachine() {}

namespace {
/// AAP Code Generator Pass Configuration Options.
class AAPPassConfig : public TargetPassConfig {
public:
  AAPPassConfig(AAPTargetMachine *TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  AAPTargetMachine &getAAPTargetMachine() const {
    return getTM<AAPTargetMachine>();
  }

  virtual bool addInstSelector();
};
} // namespace

TargetPassConfig *AAPTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AAPPassConfig(this, PM);
}

bool AAPPassConfig::addInstSelector() {
  addPass(createAAPISelDag(getAAPTargetMachine(), getOptLevel()));
  return false;
}
