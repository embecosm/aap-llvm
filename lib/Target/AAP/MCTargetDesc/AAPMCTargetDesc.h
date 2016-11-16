//===-- AAPMCTargetDesc.h - AAP Target Descriptions -------------*- C++ -*-===//
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
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_pwrite_stream;

Target &getTheAAPTarget();

MCCodeEmitter *createAAPMCCodeEmitter(MCInstrInfo const &MCII,
                                      MCRegisterInfo const &MRI,
                                      MCContext &Context);

MCAsmBackend *createAAPAsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                  const Triple &TT, StringRef CPU,
                                  const MCTargetOptions &Options);

MCObjectWriter *createAAPELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI,
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
