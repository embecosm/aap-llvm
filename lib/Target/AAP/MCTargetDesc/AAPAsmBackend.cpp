//===-- AAPAsmBackend.cpp - AAP Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAPMCTargetDesc.h"
#include "MCTargetDesc/AAPFixupKinds.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"

using namespace llvm;

namespace {

class AAPAsmBackend : public MCAsmBackend {
public:
  AAPAsmBackend(Target const &T) {}

  unsigned getNumFixupKinds() const override { return 14; }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[AAP::NumTargetFixupKinds] = {
      // We tell LLVM that branches are not PC relative to prevent it from
      // resolving them, these are more complex fields which we instead want
      // to populate in the linker.

      // This table *must* be in the order that the fixup_* kinds are defined
      // in AAPFixupKinds.h.
      //
      // Name                 offset  size  flags
      {"fixup_AAP_NONE",        0,    16,     0},
      {"fixup_AAP_BR16",        0,    9,      0},
      {"fixup_AAP_BR32",        0,    9,      0},
      {"fixup_AAP_BRCC16",      6,    3,      0},
      {"fixup_AAP_BRCC32",      6,    3,      0},
      {"fixup_AAP_BAL16",       0,    3,      0},
      {"fixup_AAP_BAL32",       0,    3,      0},
      {"fixup_AAP_ABS6",        0,    3,      0},
      {"fixup_AAP_ABS9",        0,    3,      0},
      {"fixup_AAP_ABS10",       0,    3,      0},
      {"fixup_AAP_ABS12",       0,    6,      0},
      {"fixup_AAP_ABS16",       0,    6,      0},
      {"fixup_AAP_SHIFT6",      0,    3,      0},
      {"fixup_AAP_OFF10",       0,    3,      0}
    };

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

//===-------------------------- Fixup processing --------------------------===//

  void applyFixup(MCFixup const &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value, bool IsPCRel, MCContext &Ctx) const override {
    // No target specific fixups are applied in the backend as they are all
    // handled as relocations in the linker.
    // Generic relocations are handled, as they may be literal values which
    // need to be resolved before they reach the linker.
    unsigned Size = 0;
    switch ((unsigned)Fixup.getKind()) {
    case FK_Data_1:
      Size = 1;
      break;
    case FK_Data_2:
      Size = 2;
      break;
    case FK_Data_4:
      Size = 4;
      break;
    case FK_Data_8:
      Size = 8;
      break;
    default:
      return;
    }
    for (unsigned i = 0; i < Size; ++i) {
      Data[i + Fixup.getOffset()] |= static_cast<uint8_t>(Value >> (i * 8));
    }
    return;
  }

//===------------------------ Relaxation interface ------------------------===//

  bool mayNeedRelaxation(MCInst const &Inst) const override {
    // We already generate the longest instruction necessary, so there is
    // nothing to relax.
    return false;
  }

  bool fixupNeedsRelaxation(MCFixup const &Fixup, uint64_t Value,
                            MCRelaxableFragment const *DF,
                            MCAsmLayout const &Layout) const override {
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

  void relaxInstruction(MCInst const &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {
    // The only instructions which should require relaxation are short
    // instructions with fixups, and at the moment these instructions should
    // not be selected or parsed
    llvm_unreachable("Unexpected short instruction with fixup");
  }

  bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override {
    if ((Count % 2) != 0) {
      return false;
    }

    // 0x0001 corresponds to nop $r0, 1
    for (uint64_t i = 0; i < Count; i += 2) {
      OW->write16(0x0001);
    }
    return true;
  }
};
} // end anonymous namespace

namespace {
class ELFAAPAsmBackend : public AAPAsmBackend {
  uint8_t OSABI;

public:
  ELFAAPAsmBackend(Target const &T, uint8_t OSABI)
      : AAPAsmBackend(T), OSABI(OSABI) {}

  MCObjectWriter *createObjectWriter(raw_pwrite_stream &OS) const override {
    StringRef CPU("Default");
    return createAAPELFObjectWriter(OS, OSABI, CPU);
  }
};
} // end anonymous namespace

namespace llvm {
MCAsmBackend *createAAPAsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                  const Triple &TT, StringRef CPU,
                                  const MCTargetOptions &Options) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(Triple(TT).getOS());
  return new ELFAAPAsmBackend(T, OSABI);
}
}
