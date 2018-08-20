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

using namespace llvm;

void AAPMCAsmInfo::anchor() {}

AAPMCAsmInfo::AAPMCAsmInfo(const llvm::Triple &TT) {
  CalleeSaveStackSlotSize = 2;
  // Defined as 4 bytes to ensure pointers can encode flag bits when debugging.
  CodePointerSize = 4;
  CommentString = ";";
  SupportsDebugInformation = true;
  UsesELFSectionDirectiveForBSS = true;
}
