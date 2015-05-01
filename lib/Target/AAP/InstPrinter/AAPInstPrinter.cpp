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

void AAPInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
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

void AAPInstPrinter::printMemSrcOperand(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O, const char *Modifier) {
  const MCOperand &Base = MI->getOperand(OpNo);
  const MCOperand &Offset = MI->getOperand(OpNo + 1);

  // Print register base field
  if (Base.getReg()) {
    O << getRegisterName(Base.getReg());
  }

  assert(Offset.isImm() && "Expected immediate offset field");
  if (Base.getReg()) {
    O << ',';
  }
  O << Offset.getImm();
}
