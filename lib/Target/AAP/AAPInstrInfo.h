//===-- AAPInstrInfo.h - AAP Instruction Information ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_AAPINSTRINFO_H
#define LLVM_LIB_TARGET_AAP_AAPINSTRINFO_H

#include "AAPRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "AAPGenInstrInfo.inc"

namespace llvm {
class AAPInstrInfo : public AAPGenInstrInfo {
public:
  AAPInstrInfo();
};
} // namespace llvm

#endif
