//===-- AAPMCCodeEmitter.cpp - AAP Target Descriptions --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/AAPMCCodeEmitter.h"
#include "AAP.h"
#include "MCTargetDesc/AAPFixupKinds.h"
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

void AAPMCCodeEmitter::encodeInstruction(MCInst const &MI, raw_ostream &OS,
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

  // Operand must be an expression
  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res))
    return Res;

  assert(Kind == MCExpr::SymbolRef &&
         "Currently only symbol operands are supported");

  AAP::Fixups FixupKind = AAP::Fixups(0);
  const unsigned Opcode = MI.getOpcode();
  if (Opcode == AAP::BAL) {
    FixupKind = AAP::fixup_AAP_BAL32;
  } else {
    assert(Opcode == AAP::BAL_short &&
           "Unhandled MCInst for getMachineOpValue");
    FixupKind = AAP::fixup_AAP_BAL16;
  }

  // Push the fixup, and encode 0 in the operand
  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  return 0;
}

MCCodeEmitter *llvm::createAAPMCCodeEmitter(MCInstrInfo const &MII,
                                            MCRegisterInfo const &MRI,
                                            MCContext &Context) {
  return new AAPMCCodeEmitter(MII, Context);
}

// TODO: Better way than using LUTs?
static const unsigned BRCCOpcodes[] = {
    AAP::BEQ_,       AAP::BNE_,       AAP::BLTS_,
    AAP::BLES_,      AAP::BLTU_,      AAP::BLEU_,

    AAP::BEQ_short,  AAP::BNE_short,  AAP::BLTS_short,
    AAP::BLES_short, AAP::BLTU_short, AAP::BLEU_short};

static bool findOpcode(unsigned Op, ArrayRef<unsigned> Opcodes) {
  for (auto It = Opcodes.begin(); It != Opcodes.end(); It++) {
    if (Op == *It) {
      return true;
    }
  }
  return false;
}

unsigned
AAPMCCodeEmitter::encodePCRelImmOperand(const MCInst &MI, unsigned Op,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  if (MO.isReg() || MO.isImm()) {
    return getMachineOpValue(MI, MO, Fixups, STI);
  }

  const unsigned Opcode = MI.getOpcode();
  AAP::Fixups FixupKind;
  switch (Opcode) {
  case AAP::BRA:
    FixupKind = AAP::fixup_AAP_BR32;
    break;
  case AAP::BRA_short:
    FixupKind = AAP::fixup_AAP_BR16;
    break;
  case AAP::BAL:
    FixupKind = AAP::fixup_AAP_BAL32;
    break;
  case AAP::BAL_short:
    FixupKind = AAP::fixup_AAP_BAL16;
    break;
  default:
    FixupKind = AAP::fixup_AAP_NONE;
    if (findOpcode(Opcode, BRCCOpcodes)) {
      const MCInstrDesc &Desc = MCII.get(Opcode);
      if (Desc.getSize() == 4) {
        FixupKind = AAP::fixup_AAP_BRCC32;
      } else {
        FixupKind = AAP::fixup_AAP_BRCC16;
      }
    } else {
      llvm_unreachable("Cannot encode fixup for non-branch pc-relative imm");
    }
  }
  Fixups.push_back(MCFixup::create(0, MO.getExpr(), (MCFixupKind)FixupKind));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeMemSrcOperand(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  unsigned encoding;

  // Map register number to its encoding, encode in upper 16 bits
  const MCOperand RegOp = MI.getOperand(Op);
  assert(RegOp.isReg() && "First operand is not a register!");

  unsigned Reg = MCtx.getRegisterInfo()->getEncodingValue(RegOp.getReg());
  encoding = (Reg & 0xffff) << 16;

  // Immediates map directly to their encoding, map into lower 16 bits
  MCOperand ImmOp = MI.getOperand(Op + 1);
  if (ImmOp.isImm()) {
    encoding |= static_cast<unsigned>(ImmOp.getImm()) & 0xffff;
    return encoding;
  }

  // Not an immediate, check for an expression and store as fixup to be
  // resolved later
  assert(ImmOp.isExpr() && "Second memsrc op not an immediate or expression!");

  const unsigned Opcode = MI.getOpcode();
  const MCInstrDesc &Desc = MCII.get(Opcode);
  assert(Desc.getSize() == 4 &&
         "Cannot encode an expression in a short memory+offset operand");

  AAP::Fixups FixupKind = AAP::fixup_AAP_OFF10;
  Fixups.push_back(MCFixup::create(0, ImmOp.getExpr(), MCFixupKind(FixupKind)));
  return encoding;
}

// Try to encode an immediate directly. If that is not possible then emit a
// provided fixup kind.
//
// The FixupKind is assumed to be a AAP specific fixup, if it is not then it
// signifies that a fixup cannot be emitted for the provided immediate.
unsigned AAPMCCodeEmitter::encodeImmN(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI,
                                      MCFixupKind FixupKind) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }
  assert(MO.isExpr());
  assert(MCII.get(MI.getOpcode()).getSize() == 4 &&
         "Cannot encode fixups for short instruction immediates");

  if (FixupKind >= FirstTargetFixupKind) {
    Fixups.push_back(MCFixup::create(0, MO.getExpr(), FixupKind));
  } else {
    llvm_unreachable("Cannot encode a fixup for this immediate operand");
  }
  return 0;
}

unsigned AAPMCCodeEmitter::encodeImm3(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, static_cast<MCFixupKind>(0));
}

unsigned AAPMCCodeEmitter::encodeImm6(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, MCFixupKind(AAP::fixup_AAP_ABS6));
}

unsigned AAPMCCodeEmitter::encodeImm9(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, MCFixupKind(AAP::fixup_AAP_ABS9));
}

unsigned AAPMCCodeEmitter::encodeImm10(const MCInst &MI, unsigned Op,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, MCFixupKind(AAP::fixup_AAP_ABS10));
}

unsigned AAPMCCodeEmitter::encodeImm12(const MCInst &MI, unsigned Op,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, MCFixupKind(AAP::fixup_AAP_ABS12));
}

unsigned AAPMCCodeEmitter::encodeField16(const MCInst &MI, unsigned Op,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI, MCFixupKind(AAP::fixup_AAP_ABS16));
}

unsigned AAPMCCodeEmitter::encodeShiftConst3(const MCInst &MI, unsigned Op,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    // Subtract one to get the actual encoding of the shift value
    unsigned value = MO.getImm();
    return static_cast<unsigned>(value - 1);
  } else {
    llvm_unreachable("Cannot encode an expression in a short shift operand");
  }
  return 0;
}

unsigned AAPMCCodeEmitter::encodeShiftImm6(const MCInst &MI, unsigned Op,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    // Subtract one to get the actual encoding of the shift value
    unsigned value = MO.getImm();
    return static_cast<unsigned>(value - 1);
  }
  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_SHIFT6;
  Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

#include "AAPGenMCCodeEmitter.inc"
