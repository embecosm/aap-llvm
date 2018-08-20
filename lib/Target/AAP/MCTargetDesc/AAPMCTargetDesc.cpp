//===-- AAPMCTargetDesc.cpp - AAP Target Descriptions ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides AAP specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "AAPMCTargetDesc.h"
#include "AAPMCAsmInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "AAPGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "AAPGenRegisterInfo.inc"

static MCInstrInfo *createAAPMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitAAPMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createAAPMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitAAPMCRegisterInfo(X, AAP::R0);
  return X;
}

extern "C" void LLVMInitializeAAPTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<AAPMCAsmInfo> X(getTheAAPTarget());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheAAPTarget(), createAAPMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheAAPTarget(), createAAPMCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheAAPTarget(),
                                        createAAPMCCodeEmitter);
  // Register the asm backend
  TargetRegistry::RegisterMCAsmBackend(getTheAAPTarget(), createAAPAsmBackend);
}
