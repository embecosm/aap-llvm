//===-- AAPSelectionDAGInfo.cpp - AAP SelectionDAG Info -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AAPSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AAP-selectiondag-info"
#include "AAPTargetMachine.h"
using namespace llvm;

AAPSelectionDAGInfo::AAPSelectionDAGInfo(const DataLayout &DL)
    : TargetSelectionDAGInfo(&DL) {}

AAPSelectionDAGInfo::~AAPSelectionDAGInfo() {}
