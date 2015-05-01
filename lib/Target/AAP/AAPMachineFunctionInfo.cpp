//===-- AAPMachineFuctionInfo.cpp - AAP machine function info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAPMachineFunctionInfo.h"

using namespace llvm;

void AAPMachineFunctionInfo::anchor() {}

unsigned AAPMachineFunctionInfo::getGlobalBaseReg() {
  // Return if it has already been initialized.
  if (GlobalBaseReg)
    return GlobalBaseReg;

  return GlobalBaseReg =
             MF.getRegInfo().createVirtualRegister(&AAP::GR64RegClass);
}
