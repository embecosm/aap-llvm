//===-- AAPELFObjectWriter.cpp - AAP Target Descriptions ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAPMCTargetDesc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

namespace {
class AAPELFObjectWriter : public MCELFObjectTargetWriter {
  StringRef CPU;

public:
  AAPELFObjectWriter(uint8_t OSABI, StringRef C);

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
} // namespace

AAPELFObjectWriter::AAPELFObjectWriter(uint8_t OSABI, StringRef C)
    : MCELFObjectTargetWriter(/*Is64bit*/ false, OSABI, ELF::EM_AAP,
                              /*HasRelocationAddend*/ true),
      CPU(C) {}

unsigned AAPELFObjectWriter::getRelocType(MCContext & /*Ctx*/,
                                          const MCValue & /*Target*/,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  llvm_unreachable("Unimplemented fixup kind!");
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createAAPELFObjectWriter(uint8_t OSABI, StringRef CPU) {
  return llvm::make_unique<AAPELFObjectWriter>(OSABI, CPU);
}
