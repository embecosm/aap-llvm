//===-- AAPELFObjectWriter.cpp - AAP Target Descriptions ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
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

  switch (Kind) {
  default:
    DEBUG(dbgs() << "unrecognized relocation " << Fixup.getKind() << "\n");
    llvm_unreachable("Unimplemented Fixup kind!");
    break;
  }
}

MCObjectWriter *llvm::createAAPELFObjectWriter(raw_ostream &OS, uint8_t OSABI,
                                               StringRef CPU) {
  MCELFObjectTargetWriter *MOTW = new AAPELFObjectWriter(OSABI, CPU);
  return createELFObjectWriter(MOTW, OS, /*IsLittleEndian*/ true);
}
