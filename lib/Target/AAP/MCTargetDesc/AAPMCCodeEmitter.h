//===-- AAPMCCodeEmitter.h - AAP Code Emitter -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// Definition for classes that emit AAP machine code from MCInsts
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCCODEEMITTER_H
#define LLVM_LIB_TARGET_AAP_MCTARGETDESC_AAPMCCODEEMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"

namespace llvm {
class MCFixup;
class MCInst;
class MCSubtargetInfo;
class raw_ostream;

class AAPMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MII;
  MCContext &Ctx;

public:
  AAPMCCodeEmitter(const MCInstrInfo &MII, MCContext &Ctx)
      : MII(MII), Ctx(Ctx) {}

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

//===-------------------------- Operand encoding --------------------------===//

  unsigned encodePCRelImmOperand(const MCInst &MI, unsigned Op,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned encodeMemSrcOperand(const MCInst &MI, unsigned Op,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;

  unsigned encodeImmN(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;
  unsigned encodeImm3(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;
  unsigned encodeImm6(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;
  unsigned encodeImm9(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;
  unsigned encodeImm10(const MCInst &MI, unsigned Op,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const;
  unsigned encodeImm12(const MCInst &MI, unsigned Op,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const;

  unsigned encodeField16(const MCInst &MI, unsigned Op,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;

  unsigned encodeShiftConst3(const MCInst &MI, unsigned Op,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned encodeShiftImm6(const MCInst &MI, unsigned Op,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const;

private:
  AAPMCCodeEmitter(const AAPMCCodeEmitter &) = delete;
  void operator=(const AAPMCCodeEmitter &) = delete;
}; // class AAPMCCodeEmitter

} // namespace llvm

#endif
