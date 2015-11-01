//===-- AAPSimState.h - AAP Simulator State  --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of a simulated AAP state
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAPSIMULATOR_AAPSIMSTATE_H
#define LLVM_LIB_TARGET_AAPSIMULATOR_AAPSIMSTATE_H

#include "llvm/ADT/ArrayRef.h"
#include <cstdlib>

namespace AAPSim {

/// AAPSimState - class representing processor state
class AAPSimState {
  // Registers
  uint16_t base_regs[64];
  uint32_t pc_w : 24;

  // Special registers
  uint16_t exitcode;      // Exit code register
  uint16_t overflow : 1;  // Overflow bit register

  // One namespace code memory and data memory
  uint8_t *code_memory;
  uint8_t *data_memory;
  llvm::ArrayRef<uint8_t> *code_array;

  AAPSimState(const AAPSimState&) = delete;

public:
  AAPSimState();
  ~AAPSimState();

  // Read and write the registers
  uint16_t getReg(int reg) const;
  void setReg(int reg, uint16_t val);
  uint32_t getPC() const;
  void setPC(uint32_t val_w);

  // Accesses to code memory is done 8 bits at a time to be consistent with
  // the disassembler, which takes an array of 8 bits.
  uint8_t getCodeMem(uint32_t address) const;
  void setCodeMem(uint32_t address, uint8_t val);
  llvm::ArrayRef<uint8_t> *getCodeArray() { return code_array; }

  // Read and write data memory
  uint8_t getDataMem(uint32_t address) const;
  void setDataMem(uint32_t address, uint8_t val);

  // Special register accesses
  uint16_t getExitCode() const { return exitcode; }
  void setExitCode(uint16_t code) { exitcode = code; }
  uint16_t getOverflow() const { return overflow; }
  void setOverflow(uint16_t o) { overflow = o ? 1 : 0; }
};

} // End AAPSim namespace

#endif
