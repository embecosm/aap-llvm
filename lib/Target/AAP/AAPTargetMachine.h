//===-- AAPTargetMachine.h - Define TargetMachine for AAP --------- C++ ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the AAP specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef AAP_TARGETMACHINE_H
#define AAP_TARGETMACHINE_H

#include "AAPSubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class AAPTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  AAPSubtarget Subtarget;

public:
  AAPTargetMachine(const Target &T, StringRef TT, StringRef CPU, StringRef FS,
                   const TargetOptions &Options, Reloc::Model RM,
                   CodeModel::Model CM, CodeGenOpt::Level OL);
  ~AAPTargetMachine() override;

  const AAPSubtarget *getSubtargetImpl(const Function &F) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
}; // AAPTargetMachine

} // end namespace llvm

#endif
