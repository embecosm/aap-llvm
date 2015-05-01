//===-- AAPMCAsmInfo.h - AAP asm properties --------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the AAPMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCASMINFO_H
#define LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class StringRef;

class AAPMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit AAPMCAsmInfo(StringRef TT);
};

} // namespace llvm

#endif
