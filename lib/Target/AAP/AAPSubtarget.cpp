//===-- AAPSubtarget.cpp - AAP Subtarget Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AAP specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "AAPSubtarget.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "aap-subtarget"

using namespace llvm;

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "AAPGenSubtargetInfo.inc"

void AAPSubtarget::anchor() {}

AAPSubtarget::AAPSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : AAPGenSubtargetInfo(TT, CPU, FS), FrameLowering(), InstrInfo(), RegInfo(),
      TLInfo(TM, *this), TSInfo() {}
