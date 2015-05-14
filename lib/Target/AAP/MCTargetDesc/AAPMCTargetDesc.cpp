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
#include "InstPrinter/AAPInstPrinter.h"
#include "AAPRegisterInfo.h"
#include "AAPMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
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

static MCRegisterInfo *createAAPMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitAAPMCRegisterInfo(X, AAP::R0);
  return X;
}

static MCSubtargetInfo *createAAPMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                 StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitAAPMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createAAPMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                             CodeModel::Model CM,
                                             CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
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
  RegisterMCAsmInfo<AAPMCAsmInfo> X(TheAAPTarget);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheAAPTarget, createAAPMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheAAPTarget, createAAPMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheAAPTarget, createAAPMCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(TheAAPTarget, createAAPMCCodeEmitter);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheAAPTarget,
                                          createAAPMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheAAPTarget, createAAPMCInstPrinter);

  // Register the asm backend
  TargetRegistry::RegisterMCAsmBackend(TheAAPTarget, createAAPAsmBackend);
}
