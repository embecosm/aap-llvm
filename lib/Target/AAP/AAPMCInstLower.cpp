//===-- AAPMCInstLower.cpp - Convert AAP MachineInstr to an MCInst --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower AAP MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    const AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;

  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  return MCOperand::createExpr(Expr);
}

bool llvm::LowerAAPMachineOperandToMCOperand(const MachineOperand &MO,
                                             MCOperand &OutMO,
                                             const AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_RegisterMask:
    return false;
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    OutMO = MCOperand::createReg(MO.getReg());
    return true;
  case MachineOperand::MO_Immediate:
    OutMO = MCOperand::createImm(MO.getImm());
    return true;
  case MachineOperand::MO_MachineBasicBlock:
    OutMO = lowerSymbolOperand(MO, MO.getMBB()->getSymbol(), AP);
    return true;
  case MachineOperand::MO_GlobalAddress:
    OutMO = lowerSymbolOperand(MO, AP.getSymbol(MO.getGlobal()), AP);
    return true;
  case MachineOperand::MO_ExternalSymbol:
    OutMO = lowerSymbolOperand(MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
    return true;
  case MachineOperand::MO_JumpTableIndex:
    OutMO = lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP);
    return true;
  case MachineOperand::MO_ConstantPoolIndex:
    OutMO = lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);
    return true;
  case MachineOperand::MO_BlockAddress:
    OutMO = lowerSymbolOperand(MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
    return true;
  }
}

void llvm::LowerAAPMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                        const AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;

    if (LowerAAPMachineOperandToMCOperand(MO, MCOp, AP))
      OutMI.addOperand(MCOp);
  }
}
