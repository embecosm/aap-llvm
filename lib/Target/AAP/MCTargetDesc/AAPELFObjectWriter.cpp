//===-- AAPELFObjectWriter.cpp - AAP Target Descriptions ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "MCTargetDesc/AAPFixupKinds.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "aap-elfwriter"

using namespace llvm;

namespace {

class AAPELFObjectWriter : public MCELFObjectTargetWriter {
private:
  StringRef CPU;

public:
  AAPELFObjectWriter(uint8_t OSABI, StringRef C);

  virtual unsigned GetRelocType(MCValue const &Target, MCFixup const &Fixup,
                                bool IsPCRel) const override;
};
}

AAPELFObjectWriter::AAPELFObjectWriter(uint8_t OSABI, StringRef C)
    : MCELFObjectTargetWriter(/*Is64bit*/ false, OSABI, ELF::EM_AAP,
                              /*HasRelocationAddend*/ true),
      CPU(C) {}

unsigned AAPELFObjectWriter::GetRelocType(MCValue const & /*Target*/,
                                          MCFixup const &Fixup,
                                          bool IsPCRel) const {
  llvm::MCFixupKind Kind = Fixup.getKind();

  switch ((unsigned)Kind) {
  case AAP::fixup_AAP_NONE:   return ELF::R_AAP_NONE;
  case AAP::fixup_AAP_BR32:   return ELF::R_AAP_BR32;
  case AAP::fixup_AAP_BRCC32: return ELF::R_AAP_BRCC32;
  case AAP::fixup_AAP_BAL32:  return ELF::R_AAP_BAL32;

  case AAP::fixup_AAP_ABS6:   return ELF::R_AAP_ABS6;
  case AAP::fixup_AAP_ABS9:   return ELF::R_AAP_ABS9;
  case AAP::fixup_AAP_ABS10:  return ELF::R_AAP_ABS10;
  case AAP::fixup_AAP_OFF10:  return ELF::R_AAP_OFF10;
  case AAP::fixup_AAP_ABS12:  return ELF::R_AAP_ABS12;
  case AAP::fixup_AAP_ABS16:  return ELF::R_AAP_ABS16;

  case FK_Data_1:   return ELF::R_AAP_8;
  case FK_Data_2:   return ELF::R_AAP_16;
  case FK_Data_4:   return ELF::R_AAP_32;
  case FK_Data_8:   return ELF::R_AAP_64;

  // Instrs with these fixups should never be generated or parsed, so for now
  // we should not be emitting relocations for them.
  case AAP::fixup_AAP_BR16:
  case AAP::fixup_AAP_BRCC16:
  case AAP::fixup_AAP_BAL16:
  case AAP::fixup_AAP_ABS3_SHORT:
  case AAP::fixup_AAP_ABS6_SHORT:
    llvm_unreachable("Cannot emit relocations for short instruction fixups!");
  default:
    llvm_unreachable("Unimplemented fixup kind!");
  }
  return ELF::R_AAP_NONE;
}

MCObjectWriter *llvm::createAAPELFObjectWriter(raw_ostream &OS, uint8_t OSABI,
                                               StringRef CPU) {
  MCELFObjectTargetWriter *MOTW = new AAPELFObjectWriter(OSABI, CPU);
  return createELFObjectWriter(MOTW, OS, /*IsLittleEndian*/ true);
}
