//===-- AAPAsmBackend.cpp - AAP Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAPMCTargetDesc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
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

  unsigned getNumFixupKinds() const override { return 1; }

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override {}

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
    return false;
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
