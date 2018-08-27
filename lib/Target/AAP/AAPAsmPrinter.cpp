//===-- AAPAsmPrinter.cpp - AAP LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the AAP assembly language.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "asm-printer"

using namespace llvm;


namespace {
class AAPAsmPrinter : public AsmPrinter {
public:
  AAPAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "AAP Assembly Printer"; }

  // Wrapper used by tablegen pseudo lowering implementation.
  bool lowerOperand(const MachineOperand &MO, MCOperand &OutMO);

  // Implemented by tablegen.
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);

  void EmitInstruction(const MachineInstr *MI) override;
};
} // end of anonymous namespace

bool AAPAsmPrinter::lowerOperand(const MachineOperand &MO, MCOperand &OutMO) {
  return LowerAAPMachineOperandToMCOperand(MO, OutMO, *this);
}

// Auto-generated lowering of simple pseudo instructions.
#include "AAPGenMCPseudoLowering.inc"

void AAPAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  MCInst TmpInst;
  LowerAAPMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeAAPAsmPrinter() {
  RegisterAsmPrinter<AAPAsmPrinter> X(getTheAAPTarget());
}
