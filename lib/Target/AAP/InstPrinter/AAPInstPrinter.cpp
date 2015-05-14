//===-- AAPInstPrinter.cpp - Convert AAP MCInst to assembly syntax --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an AAP MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "AAPInstPrinter.h"
#include "AAP.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#include "AAPGenAsmWriter.inc"

void AAPInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                               StringRef Annot, const MCSubtargetInfo &STI) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}


void AAPInstPrinter::printRegister(unsigned RegNo, raw_ostream &O) const {
  O << '$' << getRegisterName(RegNo);
}

void AAPInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegister(Op.getReg(), O);
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    O << *Op.getExpr();
  }
}

void AAPInstPrinter::printPCRelImmOperand(const MCInst *MI, unsigned OpNo,
                                          raw_ostream &O,
                                          const char *Modifier) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(Op.isExpr() && "unknown pcrel immediate operand");
    O << *Op.getExpr();
  }
}


void AAPInstPrinter::
printMemSrcOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                   const char *Modifier, bool WithPreDec, bool WithPostInc) {
  const MCOperand &Base = MI->getOperand(OpNo);
  const MCOperand &Offset = MI->getOperand(OpNo + 1);

  if (WithPreDec) {
    O << '-';
  }

  // Print register base field
  if (Base.getReg()) {
    printRegister(Base.getReg(), O);
  }

  if (WithPostInc) {
    O << '+';
  }

  O << ", ";
  if (Offset.isImm()) {
    O << Offset.getImm();
  }
  else if (Offset.isExpr()) {
    O << *Offset.getExpr();
  }
  else {
    llvm_unreachable("Expected immediate/expression in offset field");
  }
}

void AAPInstPrinter::
printMemSrcPostIncOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                          const char *Modifier) {
  printMemSrcOperand(MI, OpNo, O, Modifier, false, true);
}

void AAPInstPrinter::
printMemSrcPreDecOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                         const char *Modifier) {
  printMemSrcOperand(MI, OpNo, O, Modifier, true, false);
}

