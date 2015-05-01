//===-- AAPMCTargetDesc.h - AAP Target Descriptions -------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCTARGETDESC_H
#define LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class StringRef;
class Target;
class raw_ostream;

extern Target TheAAPTarget;

MCCodeEmitter *createAAPMCCodeEmitter(MCInstrInfo const &MCII,
                                      MCRegisterInfo const &MRI,
                                      MCContext &Context);

MCAsmBackend *createAAPAsmBackend(Target const &T, MCRegisterInfo const &MRI,
                                  StringRef TT, StringRef CPU);

MCObjectWriter *createAAPELFObjectWriter(raw_ostream &OS, uint8_t OSABI,
                                         StringRef CPU);

} // End llvm namespace

// Defines symbolic names for AAP registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "AAPGenRegisterInfo.inc"

// Defines symbolic names for the AAP instructions.
#define GET_INSTRINFO_ENUM
#include "AAPGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "AAPGenSubtargetInfo.inc"

#endif
