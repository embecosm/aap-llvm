//===-- AAPMCCodeEmitter.cpp - AAP Target Descriptions --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/AAPMCCodeEmitter.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "mccodemitter"

using namespace llvm;

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {
void emitLittleEndian(uint64_t Encoding, raw_ostream &OS, unsigned sz) {
  while (sz > 0) {
    OS << static_cast<uint8_t>(Encoding & 0xff);
    Encoding = Encoding >> 8;
    sz--;
  }
}
} // namespace

void AAPMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MII.get(MI.getOpcode());
  uint64_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);
  emitLittleEndian(Binary, OS, Desc.getSize());
  ++MCNumEmitted;
}

unsigned AAPMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
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
  return 0;
}

unsigned
AAPMCCodeEmitter::encodePCRelImmOperand(const MCInst &MI, unsigned Op,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

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

  unsigned Reg = Ctx.getRegisterInfo()->getEncodingValue(RegOp.getReg());
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
  const MCInstrDesc &Desc = MII.get(Opcode);
  assert(Desc.getSize() == 4 &&
         "Cannot encode an expression in a short memory+offset operand");

  return encoding;
}

// Try to encode an immediate directly.
unsigned AAPMCCodeEmitter::encodeImmN(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  const MCOperand MO = MI.getOperand(Op);
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  return 0;
}

unsigned AAPMCCodeEmitter::encodeImm3(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
}

unsigned AAPMCCodeEmitter::encodeImm6(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
}

unsigned AAPMCCodeEmitter::encodeImm9(const MCInst &MI, unsigned Op,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
}

unsigned AAPMCCodeEmitter::encodeImm10(const MCInst &MI, unsigned Op,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
}

unsigned AAPMCCodeEmitter::encodeImm12(const MCInst &MI, unsigned Op,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
}

unsigned AAPMCCodeEmitter::encodeField16(const MCInst &MI, unsigned Op,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  return encodeImmN(MI, Op, Fixups, STI);
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

  return 0;
}

MCCodeEmitter *llvm::createAAPMCCodeEmitter(const MCInstrInfo &MII,
                                            const MCRegisterInfo &MRI,
                                            MCContext &Context) {
  return new AAPMCCodeEmitter(MII, Context);
}

#include "AAPGenMCCodeEmitter.inc"
