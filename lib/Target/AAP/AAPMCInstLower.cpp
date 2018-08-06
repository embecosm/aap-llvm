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

#include "AAPMCInstLower.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

MCSymbol *
AAPMCInstLower::GetGlobalAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.getSymbol(MO.getGlobal());
}

MCSymbol *
AAPMCInstLower::GetExternalSymbolSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCSymbol *AAPMCInstLower::GetJumpTableSymbol(const MachineOperand &MO) const {
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<256> Name;
  raw_svector_ostream(Name)
      << DL.getPrivateGlobalPrefix() << "JTI" << Printer.getFunctionNumber()
      << '_' << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  // Create a symbol for the name.
  return Ctx.getOrCreateSymbol(Name);
}

MCSymbol *
AAPMCInstLower::GetConstantPoolIndexSymbol(const MachineOperand &MO) const {
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<256> Name;
  raw_svector_ostream(Name)
      << DL.getPrivateGlobalPrefix() << "CPI" << Printer.getFunctionNumber()
      << '_' << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  // Create a symbol for the name.
  return Ctx.getOrCreateSymbol(Name);
}

MCSymbol *
AAPMCInstLower::GetBlockAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.GetBlockAddressSymbol(MO.getBlockAddress());
}

MCOperand AAPMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                             MCSymbol *Sym) const {
  // FIXME: We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing.
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  return MCOperand::createExpr(Expr);
}

bool AAPMCInstLower::LowerMachineOperand(const MachineOperand &MO,
                                         MCOperand &MCOp) const {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_RegisterMask:
    return false;
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    return true;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    return true;
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = MCOperand::createExpr(
        MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
    return true;
  case MachineOperand::MO_GlobalAddress:
    MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
    return true;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO));
    return true;
  case MachineOperand::MO_JumpTableIndex:
    MCOp = LowerSymbolOperand(MO, GetJumpTableSymbol(MO));
    return true;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = LowerSymbolOperand(MO, GetConstantPoolIndexSymbol(MO));
    return true;
  case MachineOperand::MO_BlockAddress:
    MCOp = LowerSymbolOperand(MO, GetBlockAddressSymbol(MO));
    return true;
  }
}

void AAPMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;

    if (!LowerMachineOperand(MO, MCOp))
      continue;

    OutMI.addOperand(MCOp);
  }
}
