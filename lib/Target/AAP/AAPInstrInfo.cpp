//===-- AAPInstrInfo.cpp - AAP Instruction Information ----------------===//
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

#include "AAPInstrInfo.h"
#include "AAPTargetMachine.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "AAPGenInstrInfo.inc"

AAPInstrInfo::AAPInstrInfo()
    : AAPGenInstrInfo(AAP::ADJCALLSTACKDOWN, AAP::ADJCALLSTACKUP) {}
