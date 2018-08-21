//===-- AAPDisassembler.h - AAP Disassembler ----------------------*- C++ -*--//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_DISASSEMBLER_AAPDISASSEMBLER_H
#define LLVM_LIB_TARGET_AAP_DISASSEMBLER_AAPDISASSEMBLER_H

#include "llvm/MC/MCDisassembler/MCDisassembler.h"

// Pull DecodeStatus and its enum values into the global namespace.
typedef llvm::MCDisassembler::DecodeStatus DecodeStatus;

namespace llvm {
class MCInst;
class raw_ostream;

class AAPDisassembler : public MCDisassembler {
public:
  AAPDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}

  DecodeStatus getInstruction(MCInst &instr, uint64_t &size,
                              ArrayRef<uint8_t> Bytes, uint64_t address,
                              raw_ostream &vStream,
                              raw_ostream &cStream) const override;
};
} // namespace llvm

#endif
