//===-- AAPMCAsmInfo.cpp - AAP asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the AAPMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "AAPMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

void AAPMCAsmInfo::anchor() {}

AAPMCAsmInfo::AAPMCAsmInfo(StringRef TT) {
  PointerSize = CalleeSaveStackSlotSize = 2;
  CommentString = ";";
  UsesELFSectionDirectiveForBSS = true;
}
