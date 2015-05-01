//===-- AAPMCCodeEmitter.cpp - AAP Target Descriptions --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "MCTargetDesc/AAPFixupKinds.h"
#include "MCTargetDesc/AAPMCCodeEmitter.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "aap-codemitter"

using namespace llvm;
using namespace AAP;

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {
void emitLittleEndian(uint64_t Encoding, raw_ostream &OS, unsigned sz) {
  while (sz > 0) {
    OS << static_cast<uint8_t>(Encoding & 0xff);
    Encoding = Encoding >> 8;
    sz--;
  }
}
}

AAPMCCodeEmitter::AAPMCCodeEmitter(MCInstrInfo const &MII, MCContext &Context)
    : MCtx(Context), MCII(MII) {}

void AAPMCCodeEmitter::EncodeInstruction(MCInst const &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         MCSubtargetInfo const &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);
  emitLittleEndian(Binary, OS, Desc.getSize());
  ++MCNumEmitted;
}

unsigned AAPMCCodeEmitter::getMachineOpValue(MCInst const &MI,
                                             MCOperand const &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             MCSubtargetInfo const &STI) const {
  if (MO.isReg())
    return MCtx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // MO must be an expression
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr *>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert(Kind == MCExpr::SymbolRef);

  AAP::Fixups FixupKind = AAP::Fixups(0);

  // switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
  //  default: llvm_unreachable("Unknown fixup kind!");
  //}

  // Push fixup, encoding 0 as the current operand
  Fixups.push_back(
      MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind), MI.getLoc()));
  return 0;
}

MCCodeEmitter *llvm::createAAPMCCodeEmitter(MCInstrInfo const &MII,
                                            MCRegisterInfo const &MRI,
                                            MCContext &Context) {
  return new AAPMCCodeEmitter(MII, Context);
}

unsigned
AAPMCCodeEmitter::encodePCRelImmOperand(const MCInst &MI, unsigned Op,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  if (MO.isReg() || MO.isImm()) {
    return getMachineOpValue(MI, MO, Fixups, STI);
  }

  AAP::Fixups FixupKind;
  switch (MI.getOpcode()) {
  default:
    FixupKind = AAP::fixup_AAP_BR32;
    break;
  }
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), (MCFixupKind)FixupKind));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeMemSrcOperand(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  unsigned encoding;
  const MCOperand reg = MI.getOperand(Op);
  assert(reg.isReg() && "First operand is not a register!");

  // Register number map directly to their encoding
  encoding = (reg.getReg() & 0x3f) << 16;
  MCOperand imm = MI.getOperand(Op + 1);

  if (imm.isImm()) {
    encoding |= (static_cast<short>(imm.getImm()) & 0xffff);
  } else {
    // Not yet an immediate, check for an expression and store as fixup to
    // be resolved later
    assert(imm.isExpr() && "Second operand is not an immediate or expression!");

    llvm_unreachable("Not yet implemented fixup");
    // AAP::Fixups FixupKind = AAP::fixup_AAP_HIGH16;
    // Fixups.push_back(MCFixup::Create(0, imm.getExpr(),
    // MCFixupKind(FixupKind)));
  }
  return encoding;
}

#include "AAPGenMCCodeEmitter.inc"
