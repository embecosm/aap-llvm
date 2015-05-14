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

using namespace llvm;

namespace {

class AAPAsmBackend : public MCAsmBackend {
public:
  AAPAsmBackend(Target const &T) {}

  unsigned getNumFixupKinds() const override { return 14; }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[AAP::NumTargetFixupKinds] = {
      // This table *must* be in the order that the fixup_* kinds are defined
      // in
      // AAPFixupKinds.h.
      //
      // Name                 offset  size  flags
      {"fixup_AAP_NONE",        0,    16,     0},
      {"fixup_AAP_BR16",        0,    9,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_BR32",        0,    9,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_BRCC16",      6,    3,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_BRCC32",      6,    3,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_BAL16",       0,    3,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_BAL32",       0,    3,      MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_AAP_ABS3_SHORT",  0,    3,      0},
      {"fixup_AAP_ABS6_SHORT",  0,    6,      0},
      {"fixup_AAP_ABS6",        0,    3,      0},
      {"fixup_AAP_ABS9",        0,    3,      0},
      {"fixup_AAP_ABS10",       0,    3,      0},
      {"fixup_AAP_ABS12",       0,    6,      0},
      {"fixup_AAP_ABS16",       0,    6,      0}
    };

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

  void applyFixup(MCFixup const &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value, bool IsPCRel) const override {
    return;
  }

  bool mayNeedRelaxation(MCInst const &Inst) const override { return false; }

  bool fixupNeedsRelaxation(MCFixup const &Fixup, uint64_t Value,
                            MCRelaxableFragment const *DF,
                            MCAsmLayout const &Layout) const override {
    llvm_unreachable("fixupNeedsRelaxation() unimplemented");
  }

  void relaxInstruction(MCInst const &Inst, MCInst &Res) const override {
    llvm_unreachable("relaxInstruction() unimplemented");
  }

  bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override {
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

  MCObjectWriter *createObjectWriter(raw_ostream &OS) const override {
    StringRef CPU("Default");
    return createAAPELFObjectWriter(OS, OSABI, CPU);
  }
};
} // end anonymous namespace

namespace llvm {
MCAsmBackend *createAAPAsmBackend(Target const &T, MCRegisterInfo const &MRI,
                                  StringRef TT, StringRef CPU) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(Triple(TT).getOS());
  return new ELFAAPAsmBackend(T, OSABI);
}
}
