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
  COND_GTS = 3,
  COND_LTU = 4,
  COND_GTU = 5,
  COND_INVALID = -1
};
} // end namespace AAP

namespace llvm {
extern Target TheAAPTarget;
class AAPTargetMachine;

FunctionPass *createAAPISelDag(AAPTargetMachine &TM,
                               CodeGenOpt::Level OptLevel);

} // end namespace llvm

#endif
