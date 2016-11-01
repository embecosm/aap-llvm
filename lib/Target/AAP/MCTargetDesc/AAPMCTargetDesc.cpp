//===-- AAPMCTargetDesc.cpp - AAP Target Descriptions ---------------===//
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
#include "InstPrinter/AAPInstPrinter.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "AAPGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "AAPGenSubtargetInfo.inc"

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

static MCSubtargetInfo *createAAPMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createAAPMCSubtargetInfoImpl(TT, CPU, FS);
}

static void adjustCodeGenOpts(const Triple &TT, Reloc::Model RM,
                              CodeModel::Model &CM) {
  return;
}

static MCInstPrinter *createAAPMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new AAPInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" void LLVMInitializeAAPTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<AAPMCAsmInfo> X(getTheAAPTarget());

  // Register the MC codegen info.
  TargetRegistry::registerMCAdjustCodeGenOpts(getTheAAPTarget(),
                                              adjustCodeGenOpts);
  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheAAPTarget(), createAAPMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheAAPTarget(), createAAPMCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheAAPTarget(),
                                        createAAPMCCodeEmitter);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheAAPTarget(),
                                          createAAPMCSubtargetInfo);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheAAPTarget(),
                                        createAAPMCInstPrinter);
  // Register the asm backend
  TargetRegistry::RegisterMCAsmBackend(getTheAAPTarget(), createAAPAsmBackend);
}
