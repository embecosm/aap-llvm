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

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;

Target &getTheAAPTarget();

MCCodeEmitter *createAAPMCCodeEmitter(const MCInstrInfo &MII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Context);

MCAsmBackend *createAAPAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createAAPELFObjectWriter(uint8_t OSABI,
                                                               StringRef CPU);

} // namespace llvm

// Defines symbolic names for AAP registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "AAPGenRegisterInfo.inc"

// Defines symbolic names for the AAP instructions.
#define GET_INSTRINFO_ENUM
#include "AAPGenInstrInfo.inc"

#endif
