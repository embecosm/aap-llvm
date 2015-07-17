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
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"

using namespace llvm;

namespace {

class AAPAsmBackend : public MCAsmBackend {
public:
  AAPAsmBackend(Target const &T) {}

  unsigned getNumFixupKinds() const override { return 15; }

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
      {"fixup_AAP_OFF10",       0,    3,      0},
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

  bool mayNeedRelaxation(MCInst const &Inst) const override {
    switch (Inst.getOpcode()) {
    case AAP::NOP_short:
    case AAP::ADD_i3_short:
    case AAP::SUB_i3_short:
    case AAP::ASR_i3_short:
    case AAP::LSL_i3_short:
    case AAP::LSR_i3_short:
    case AAP::MOV_i6_short:
    case AAP::LDB_short:
    case AAP::LDW_short:
    case AAP::LDB_postinc_short:
    case AAP::LDW_postinc_short:
    case AAP::LDB_predec_short:
    case AAP::LDW_predec_short:
    case AAP::STB_short:
    case AAP::STW_short:
    case AAP::STB_postinc_short:
    case AAP::STW_postinc_short:
    case AAP::STB_predec_short:
    case AAP::STW_predec_short:
    case AAP::BRA_short:
    case AAP::BAL_short:
    case AAP::BEQ_short:
    case AAP::BNE_short:
    case AAP::BLTS_short:
    case AAP::BGTS_short:
    case AAP::BLTU_short:
    case AAP::BGTU_short:
      return true;
    default:
      return false;
    }
  }

  bool fixupNeedsRelaxation(MCFixup const &Fixup, uint64_t Value,
                            MCRelaxableFragment const *DF,
                            MCAsmLayout const &Layout) const override {
    // All instructions with short fixups should be relaxed
    switch((unsigned)Fixup.getKind()) {
    case AAP::fixup_AAP_BR16:
    case AAP::fixup_AAP_BRCC16:
    case AAP::fixup_AAP_BAL16:
    case AAP::fixup_AAP_ABS3_SHORT:
    case AAP::fixup_AAP_ABS6_SHORT:
      return true;
    default:
      return false;
    }
  }


  // Get the equivalent 
  static unsigned getRelaxedOpcode(unsigned Opcode) {
    switch (Opcode) {
    case AAP::NOP_short:          return AAP::NOP;
    case AAP::ADD_i3_short:       return AAP::ADD_i10;
    case AAP::SUB_i3_short:       return AAP::SUB_i10;
    case AAP::ASR_i3_short:       return AAP::ASR_i6;
    case AAP::LSL_i3_short:       return AAP::LSL_i6;
    case AAP::LSR_i3_short:       return AAP::LSR_i6;
    case AAP::MOV_i6_short:       return AAP::MOV_i16;
    case AAP::LDB_short:          return AAP::LDB;
    case AAP::LDW_short:          return AAP::LDW;
    case AAP::LDB_postinc_short:  return AAP::LDB_postinc;
    case AAP::LDW_postinc_short:  return AAP::LDW_postinc;
    case AAP::LDB_predec_short:   return AAP::LDB_predec;
    case AAP::LDW_predec_short:   return AAP::LDW_predec;
    case AAP::STB_short:          return AAP::STB;
    case AAP::STW_short:          return AAP::STW;
    case AAP::STB_postinc_short:  return AAP::STB_postinc;
    case AAP::STW_postinc_short:  return AAP::STW_postinc;
    case AAP::STB_predec_short:   return AAP::STB_predec;
    case AAP::STW_predec_short:   return AAP::STW_predec;
    case AAP::BRA_short:          return AAP::BRA;
    case AAP::BAL_short:          return AAP::BAL;
    case AAP::BEQ_short:          return AAP::BEQ_;
    case AAP::BNE_short:          return AAP::BNE_;
    case AAP::BLTS_short:         return AAP::BLTS_;
    case AAP::BGTS_short:         return AAP::BGTS_;
    case AAP::BLTU_short:         return AAP::BLTU_;
    case AAP::BGTU_short:         return AAP::BGTU_;
    default:
      llvm_unreachable("Unknown opcode for relaxation!");
    }
    return 0;
  }

  void relaxInstruction(MCInst const &Inst, MCInst &Res) const override {
    // Relax all short instructions to their equivalent long instruction
    Res = Inst;
    Res.setOpcode(getRelaxedOpcode(Inst.getOpcode()));
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
