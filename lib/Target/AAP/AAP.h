//===-- AAP.h - Top-level interface for AAP representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// AAP back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_AAP_H
#define TARGET_AAP_H

#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace AAPCC {
// AAP specific condition codes
enum CondCode {
  COND_EQ = 0,
  COND_NE = 1,
  COND_LTS = 2,
  COND_LES = 3,
  COND_LTU = 4,
  COND_LEU = 5,
  COND_INVALID = -1
};
} // end namespace AAP

namespace llvm {
extern Target TheAAPTarget;
class AAPTargetMachine;

FunctionPass *createAAPISelDag(AAPTargetMachine &TM,
                               CodeGenOpt::Level OptLevel);

FunctionPass *createAAPShortInstrPeepholePass(AAPTargetMachine &TM);

namespace AAP {
// Various helper methods to define operand ranges used throughout the backend
static bool inline isImm3(int64_t I) { return (I >= 0) && (I <= 7); }
static bool inline isImm6(int64_t I) { return (I >= 0) && (I <= 63); }
static bool inline isImm9(int64_t I) { return (I >= 0) && (I <= 511); }
static bool inline isImm10(int64_t I) { return (I >= 0) && (I <= 1023); }
static bool inline isImm12(int64_t I) { return (I >= 0) && (I <= 4095); }
static bool inline isImm16(int64_t I) { return (I >= -32768) && (I <= 65535); }

static bool inline isOff3(int64_t I) { return (I >= -4) && (I <= 3); }
static bool inline isOff6(int64_t I) { return (I >= -32) && (I <= 31); }
static bool inline isOff9(int64_t I) { return (I >= -256) && (I <= 255); }
static bool inline isOff10(int64_t I) { return (I >= -512) && (I <= 511); }

static bool inline isShiftImm3(int64_t I) { return (I >= 1) && (I <= 8); }
static bool inline isShiftImm6(int64_t I) { return (I >= 1) && (I <= 64); }
} // end of namespace AAP
} // end namespace llvm

#endif
