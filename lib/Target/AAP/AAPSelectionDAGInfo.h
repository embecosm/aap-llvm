//===-- AAPSelectionDAGInfo.h - AAP SelectionDAG Info -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AAP subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef AAPSELECTIONDAGINFO_H
#define AAPSELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class AAPTargetMachine;

class AAPSelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  explicit AAPSelectionDAGInfo();
  ~AAPSelectionDAGInfo();
};
}

#endif
