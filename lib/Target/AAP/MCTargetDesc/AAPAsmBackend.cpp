//===-- AAPAsmBackend.cpp - AAP Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAPMCTargetDesc.h"
#include "AAPFixupKinds.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

namespace {
class AAPAsmBackend : public MCAsmBackend {
  uint8_t OSABI;

public:
  AAPAsmBackend(uint8_t OSABI) : MCAsmBackend(support::little), OSABI(OSABI) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

//===-------------------------- Fixup processing --------------------------===//

  unsigned getNumFixupKinds() const override {
    return AAP::NumTargetFixupKinds;
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

//===------------------------ Relaxation interface ------------------------===//

  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override {
    // We already generate the longest instruction necessary, so there is
    // nothing to relax.
    return false;
  }

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    // We already generate the longest instruction necessary so there is
    // no need to relax, and at the moment we should never see fixups for
    // short instructions.
    switch ((unsigned)Fixup.getKind()) {
    case AAP::fixup_AAP_BR16:
    case AAP::fixup_AAP_BRCC16:
    case AAP::fixup_AAP_BAL16:
      llvm_unreachable("Cannot relax short instruction fixups!");
    default:
      return false;
    }
  }

  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {
    // The only instructions which should require relaxation are short
    // instructions with fixups, and at the moment these instructions should
    // not be selected or parsed
    llvm_unreachable("Unexpected short instruction with fixup");
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;
};
} // end anonymous namespace

bool AAPAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  if ((Count % 2) != 0)
    return false;

  // 0x0001 corresponds to nop $r0, 1
  for (uint64_t i = 0; i < Count; i += 2) {
    OS.write(0x00);
    OS.write(0x01);
  }
  return true;
}

const MCFixupKindInfo &AAPAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[AAP::NumTargetFixupKinds] = {
    // We tell LLVM that branches are not PC relative to prevent it from
    // resolving them, these are more complex fields which we instead want
    // to populate in the linker.
    // MCAsmStreamer assumes contiguous fixups, so for AAP's non-contiguous
    // fixups the offset+size is extended to the next byte.

    // This table *must* be in the order that the fixup_* kinds are defined
    // in AAPFixupKinds.h.
    //
    // Name                 offset  size  flags
    {"fixup_AAP_NONE",        0,    16,     0},
    {"fixup_AAP_BR16",        0,    9,      0},
    {"fixup_AAP_BR32",        0,    32,     0},
    {"fixup_AAP_BRCC16",      6,    3,      0},
    {"fixup_AAP_BRCC32",      6,    26,     0},
    {"fixup_AAP_BAL16",       3,    6,      0},
    {"fixup_AAP_BAL32",       3,    29,     0},
    {"fixup_AAP_ABS6",        0,    24,     0},
    {"fixup_AAP_ABS9",        0,    32,     0},
    {"fixup_AAP_ABS10",       0,    32,     0},
    {"fixup_AAP_ABS12",       0,    24,     0},
    {"fixup_AAP_ABS16",       0,    32,     0},
    {"fixup_AAP_SHIFT6",      0,    24,     0},
    {"fixup_AAP_OFF10",       0,    32,     0}
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
          "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

// Return an adjusted value according to its target fixup.
static uint64_t adjustFixupValue(MCFixupKind FixupKind, uint64_t Value) {
  switch ((unsigned)FixupKind) {
  default:
    // Fixups for short instructions are unimplemented as they are not selected.
    llvm_unreachable("Unimplemented fixup kind");
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return Value;
  case AAP::fixup_AAP_ABS6:
  case AAP::fixup_AAP_SHIFT6:
    // Inst_rr_i6
    return (((Value >> 0) & 0x07) << 0) |
           (((Value >> 3) & 0x07) << 16);
  case AAP::fixup_AAP_ABS9:
    // Inst_rr_i9
    return (((Value >> 0) & 0x07) << 0) |
           (((Value >> 3) & 0x07) << 16) |
           (((Value >> 6) & 0x07) << 26);
  case AAP::fixup_AAP_ABS10:
  case AAP::fixup_AAP_OFF10:
    // Inst_rr_i10
    return (((Value >> 0) & 0x07) << 0) |
           (((Value >> 3) & 0x07) << 16) |
           (((Value >> 6) & 0x0F) << 25);
  case AAP::fixup_AAP_ABS12:
    // Inst_r_i12
    return (((Value >> 0) & 0x3F) << 0) |
           (((Value >> 6) & 0x3F) << 16);
  case AAP::fixup_AAP_ABS16:
    // Inst_r_i16
    return (((Value >> 0) & 0x3F) << 0) |
           (((Value >> 6) & 0x3F) << 16) |
           (((Value >> 12) & 0x0F) << 25);
  case AAP::fixup_AAP_BRCC32:
    // Inst_i10_rr
    return (((Value >> 0) & 0x07) << 0) |
           (((Value >> 3) & 0x7F) << 16);
  case AAP::fixup_AAP_BAL32:
    // Inst_i16_r
    return (((Value >> 0) & 0x3F) << 0) |
           (((Value >> 6) & 0x03FF) << 19);
  case AAP::fixup_AAP_BR32:
    // Inst_i22
    return (((Value >> 0) & 0x01FF) << 0) |
           (((Value >> 9) & 0x1FFF) << 16);
  }
}

void AAPAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target,
                               MutableArrayRef<char> Data, uint64_t Value,
                               bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  // No target specific fixups are applied in the backend as they are all
  // handled as relocations in the linker.
  // Generic relocations are handled, as they may be literal values which
  // need to be resolved before they reach the linker.
  MCFixupKind Kind = Fixup.getKind();
  MCFixupKindInfo Info = getFixupKindInfo(Kind);
  Value = adjustFixupValue(Kind, Value);
  Value <<= Info.TargetOffset;

  unsigned NumBytes = (Info.TargetSize + Info.TargetOffset + 7) / 8;
  for (unsigned i = 0; i < NumBytes; i++)
    Data[i + Fixup.getOffset()] |= static_cast<uint8_t>(Value >> (i * 8));
  return;
}

std::unique_ptr<MCObjectTargetWriter>
AAPAsmBackend::createObjectTargetWriter() const {
  StringRef CPU("Default");
  return createAAPELFObjectWriter(OSABI, CPU);
}

MCAsmBackend *llvm::createAAPAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options) {
  uint8_t OSABI =
      MCELFObjectTargetWriter::getOSABI(Triple(STI.getTargetTriple()).getOS());
  return new AAPAsmBackend(OSABI);
}
