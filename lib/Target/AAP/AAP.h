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

#ifndef LLVM_LIB_TARGET_AAP_AAP_H
#define LLVM_LIB_TARGET_AAP_AAP_H

#include "MCTargetDesc/AAPMCTargetDesc.h"

namespace llvm {
class AAPTargetMachine;
class AsmPrinter;
class FunctionPass;
class MachineInstr;
class MachineOperand;
class MCInst;
class MCOperand;

bool LowerAAPMachineOperandToMCOperand(const MachineOperand &MO,
                                       MCOperand &OutMO, const AsmPrinter &AP);

void LowerAAPMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                  const AsmPrinter &AP);

FunctionPass *createAAPISelDag(AAPTargetMachine &TM);
}

#endif
