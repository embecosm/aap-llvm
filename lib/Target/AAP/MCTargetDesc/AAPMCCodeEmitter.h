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

#ifndef AAPMCCODEEMITTER_H
#define AAPMCCODEEMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

class AAPMCCodeEmitter : public MCCodeEmitter {
  MCContext &MCtx;
  MCInstrInfo const &MCII;

public:
  AAPMCCodeEmitter(MCInstrInfo const &aMII, MCContext &aMCT);

  MCSubtargetInfo const &getSubtargetInfo() const;

  void EncodeInstruction(MCInst const &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         MCSubtargetInfo const &STI) const override;

  unsigned encodePCRelImmOperand(const MCInst &MI, unsigned Op,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned encodeMemSrcOperand(const MCInst &MI, unsigned Op,
                               SmallVectorImpl<MCFixup> &Fixups,
                               MCSubtargetInfo const &STI) const;

  unsigned encodeImm3(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      MCSubtargetInfo const &STI) const;
  unsigned encodeImm6(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      MCSubtargetInfo const &STI) const;
  unsigned encodeImm9(const MCInst &MI, unsigned Op,
                      SmallVectorImpl<MCFixup> &Fixups,
                      MCSubtargetInfo const &STI) const;
  unsigned encodeImm10(const MCInst &MI, unsigned Op,
                       SmallVectorImpl<MCFixup> &Fixups,
                       MCSubtargetInfo const &STI) const;
  unsigned encodeImm12(const MCInst &MI, unsigned Op,
                       SmallVectorImpl<MCFixup> &Fixups,
                       MCSubtargetInfo const &STI) const;
  unsigned encodeImm16(const MCInst &MI, unsigned Op,
                       SmallVectorImpl<MCFixup> &Fixups,
                       MCSubtargetInfo const &STI) const;

  // TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(MCInst const &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 MCSubtargetInfo const &STI) const;

  /// Return binary encoding of operand.
  unsigned getMachineOpValue(MCInst const &MI, MCOperand const &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             MCSubtargetInfo const &STI) const;

private:
  AAPMCCodeEmitter(AAPMCCodeEmitter const &) = delete;
  void operator=(AAPMCCodeEmitter const &) = delete;
}; // class AAPMCCodeEmitter

} // namespace llvm

#endif /* AAPMCCODEEMITTER_H */
