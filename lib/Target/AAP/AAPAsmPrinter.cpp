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
#include "InstPrinter/AAPInstPrinter.h"
#include "AAPInstrInfo.h"
#include "AAPMCInstLower.h"
#include "AAPTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class AAPAsmPrinter : public AsmPrinter {
public:
  AAPAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  const char *getPassName() const override { return "AAP Assembly Printer"; }

  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O,
                    const char *Modifier = nullptr);
  void EmitInstruction(const MachineInstr *MI) override;
};
} // end of anonymous namespace

void AAPAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                 raw_ostream &O, const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  default:
    llvm_unreachable("Not implemented yet!");
  case MachineOperand::MO_Register:
    O << AAPInstPrinter::getRegisterName(MO.getReg());
    return;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    return;
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    return;
  case MachineOperand::MO_GlobalAddress: {
    bool isMemOp = Modifier && !strcmp(Modifier, "mem");
    uint64_t Offset = MO.getOffset();

    // If the global address expression is a part of displacement field with a
    // register base, we should not emit any prefix symbol here, e.g.
    //   mov.w &foo, r1
    // vs
    //   mov.w glb(r1), r2
    // Otherwise (!) msp430-as will silently miscompile the output :(
    O << (isMemOp ? '&' : '#');
    if (Offset)
      O << '(' << Offset << '+';

    O << *getSymbol(MO.getGlobal());

    if (Offset)
      O << ')';

    return;
  }
  }
}

void AAPAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  AAPMCInstLower MCInstLowering(OutContext, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeAAPAsmPrinter() {
  RegisterAsmPrinter<AAPAsmPrinter> X(TheAAPTarget);
}
