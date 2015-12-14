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

enum SimStatus {
  SIM_OK,           // Instruction executed
  SIM_INVALID_INSN, // Invalid instruction
  SIM_BREAKPOINT,   // Simulator hit a breakpoint
  SIM_QUIT,         // Signal to exit linear simulator
  SIM_TRAP,         // General trap signal
  SIM_EXCEPT_MEM,   // Invalid memory exception
  SIM_EXCEPT_REG,   // Invalid register exception
  SIM_TIMEOUT       // Simulator timeout
};

/// AAPSimState - class representing processor state
class AAPSimState {
  // Registers
  uint16_t base_regs[64];
  uint32_t pc_w : 24;

  // Special registers
  uint16_t exitcode;      // Exit code register
  uint16_t overflow : 1;  // Overflow bit register
  SimStatus status;       // Simulator status

  // One namespace code memory and data memory
  uint8_t *code_memory;
  uint8_t *data_memory;
  llvm::ArrayRef<uint8_t> *code_array;

  bool debug_trace;

  AAPSimState(const AAPSimState&) = delete;


public:
  AAPSimState();
  ~AAPSimState();

  // Read and write the registers
  uint16_t getReg(int reg);
  void setReg(int reg, uint16_t val);
  uint32_t getPC();
  void setPC(uint32_t val_w);
  unsigned int getNumRegs() { return 64; }

  // Accesses to code memory is done 8 bits at a time to be consistent with
  // the disassembler, which takes an array of 8 bits.
  uint8_t getCodeMem(uint32_t address);
  void setCodeMem(uint32_t address, uint8_t val);
  llvm::ArrayRef<uint8_t> *getCodeArray() { return code_array; }

  // Read and write data memory
  uint8_t getDataMem(uint32_t address);
  void setDataMem(uint32_t address, uint8_t val);

  // Special register accesses
  uint16_t getExitCode() const { return exitcode; }
  void setExitCode(uint16_t code) { exitcode = code; }
  uint16_t getOverflow() const { return overflow; }
  void setOverflow(uint16_t o) { overflow = o ? 1 : 0; }

  // Get and reset exception state
  SimStatus getStatus() { return status; }
  void resetStatus() { status = SimStatus::SIM_OK; }

  // Trace control
  bool getTracing() const { return debug_trace; }
  void setTracing(bool enabled) { debug_trace = enabled; }
};

} // End AAPSim namespace

#endif
