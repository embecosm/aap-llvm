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
  const unsigned Opcode = MI.getOpcode();
  if (Opcode == AAP::BAL) {
    FixupKind = AAP::fixup_AAP_BAL32;
  }
  else if (Opcode == AAP::BAL_short) {
    FixupKind = AAP::fixup_AAP_BAL16;
  }
  else {
    llvm_unreachable("Unknown fixup kind!");
  }

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


// TODO: Better way than using LUTs?
static const unsigned BRCCOpcodes[] = {
  AAP::BEQ_,  AAP::BNE_,
  AAP::BLTS_, AAP::BGTS_,
  AAP::BLTU_, AAP::BGTU_,

  AAP::BEQ_short,  AAP::BNE_short,
  AAP::BLTS_short, AAP::BGTS_short,
  AAP::BLTU_short, AAP::BGTU_short
};
static const unsigned LoadStoreOpcodes[] = {
  AAP::LDB, AAP::LDW, //AAP::LDD,
  AAP::LDB_postinc, AAP::LDW_postinc, //AAP::LDD_postinc,
  AAP::LDB_predec, AAP::LDW_predec, //AAP::LDD_predec,

  AAP::STB, AAP::STW, //AAP::STD,
  AAP::STB_postinc, AAP::STW_postinc, //AAP::STD_postinc,
  AAP::STB_predec, AAP::STW_predec, //AAP::STD_predec,

  AAP::LDB_short, AAP::LDW_short,
  AAP::LDB_postinc_short, AAP::LDW_postinc_short,
  AAP::LDB_predec_short,  AAP::LDW_predec_short,

  AAP::STB_short, AAP::STW_short,
  AAP::STB_postinc_short, AAP::STW_postinc_short,
  AAP::STB_predec_short,  AAP::STW_predec_short,
};

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
    if (findOpcode(Opcode, BRCCOpcodes)) {
      const MCInstrDesc& Desc = MCII.get(Opcode);
      if (Desc.getSize() == 4) {
        FixupKind = AAP::fixup_AAP_BRCC32;
      }
      else {
        FixupKind = AAP::fixup_AAP_BRCC16;
      }
    }
  }
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), (MCFixupKind)FixupKind));
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

  AAP::Fixups FixupKind;
  const unsigned Opcode = MI.getOpcode();
  if (findOpcode(Opcode, LoadStoreOpcodes)) {
    // Fixup to emit depends on whether we're encoding a memsrc for a
    // short or normal length instruction
    const MCInstrDesc& Desc = MCII.get(Opcode);
    FixupKind = 
      (Desc.getSize() == 4) ? AAP::fixup_AAP_ABS6 : AAP::fixup_AAP_ABS3_SHORT;
  }
  else {
    llvm_unreachable("Unknown opcode!");
  }
  Fixups.push_back(MCFixup::Create(0, ImmOp.getExpr(), MCFixupKind(FixupKind)));
  return encoding;
}

unsigned
AAPMCCodeEmitter::encodeImm3(const MCInst &MI, unsigned Op,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_ABS3_SHORT;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeImm6(const MCInst &MI, unsigned Op,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());

  const MCInstrDesc& Desc = MCII.get(MI.getOpcode());
  AAP::Fixups FixupKind =
    (Desc.getSize() == 4) ? AAP::fixup_AAP_ABS6 : AAP::fixup_AAP_ABS6_SHORT;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeImm9(const MCInst &MI, unsigned Op,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_ABS9;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeImm10(const MCInst &MI, unsigned Op,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_ABS10;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeImm12(const MCInst &MI, unsigned Op,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_ABS12;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned
AAPMCCodeEmitter::encodeImm16(const MCInst &MI, unsigned Op,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr());
  AAP::Fixups FixupKind = AAP::fixup_AAP_ABS16;
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}


#include "AAPGenMCCodeEmitter.inc"
