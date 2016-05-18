//===-- AAPSimState.cpp - AAP Simulator State  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides the implementation of a simulated AAP state
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "AAPSimState.h"
#include <cassert>

using namespace AAPSim;

AAPSimState::AAPSimState() {
  for (int i = 0; i < 64; ++i)
    base_regs[i] = 0;
  pc_w = 0;

  // Code memory is 24-bit word addressed
  code_memory = new uint8_t[0x1ffffff];
  assert(code_memory && "Unable to allocate code memory");

  code_array = new llvm::ArrayRef<uint8_t>(code_memory, 0x1ffffff);

  // Data memory is 16-bit byte addressed
  data_memory = new uint8_t[0xffff];
  assert(data_memory && "Unable to allocate data memory");

  // We haven't hit any exception yet
  status = SimStatus::SIM_OK;

  debug_trace = false;
}

AAPSimState::~AAPSimState() {
  delete code_memory;
  delete data_memory;
}

uint16_t AAPSimState::getReg(int reg) {
  if (reg >= 64) {
    status = SimStatus::SIM_EXCEPT_REG;
    return 0xffff;
  }
  if (debug_trace)
    llvm::dbgs() << " REG READ: " << reg << ": 0x"
                 << llvm::format("%04" PRIx64, base_regs[reg]) << "\n";
  return base_regs[reg];
}

void AAPSimState::setReg(int reg, uint16_t val) {
  if (reg >= 64) {
    status = SimStatus::SIM_EXCEPT_REG;
    return;
  }
  if (debug_trace)
    llvm::dbgs() << " REG WRITE: " << reg << ": 0x"
                 << llvm::format("%04" PRIx64, val) << "\n";
  base_regs[reg] = val;
}

uint32_t AAPSimState::getPC() {
  return pc_w;
}

void AAPSimState::setPC(uint32_t val_w) {
  if (val_w > 0xffffff) {
    status = SimStatus::SIM_EXCEPT_REG;
    return;
  }
  pc_w = val_w;
}

uint8_t AAPSimState::getCodeMem(uint32_t address) {
  if (address > 0x1ffffff) {
    status = SimStatus::SIM_EXCEPT_MEM;
    return 0xff;
  }
  if (debug_trace)
    llvm::dbgs() << " CODEMEM READ: 0x"
                 << llvm::format("%07" PRIx64, address) << ": 0x"
                 << llvm::format("%02" PRIx64, code_memory[address]) << "\n";
  return code_memory[address];
}

void AAPSimState::setCodeMem(uint32_t address, uint8_t val) {
  if (address > 0x1ffffff) {
    status = SimStatus::SIM_EXCEPT_MEM;
    return;
  }
  if (debug_trace)
    llvm::dbgs() << " CODEMEM WRITE: 0x"
                 << llvm::format("%07" PRIx64, address) << ": 0x"
                 << llvm::format("%02" PRIx64, val) << "\n";
  code_memory[address] = val;
}

uint8_t AAPSimState::getDataMem(uint32_t address) {
    if (address > 0xffff) {
    status = SimStatus::SIM_EXCEPT_MEM;
    return 0xff;
  }
  if (debug_trace)
    llvm::dbgs() << " DATAMEM READ: 0x"
                 << llvm::format("%04" PRIx64, address) << ": 0x"
                 << llvm::format("%02" PRIx64, code_memory[address]) << "\n";
  return data_memory[address];
}

void AAPSimState::setDataMem(uint32_t address, uint8_t val) {
  if (address > 0xffff) {
    status = SimStatus::SIM_EXCEPT_MEM;
    return;
  }
  if (debug_trace)
    llvm::dbgs() << " DATAMEM WRITE: 0x"
                 << llvm::format("%04" PRIx64, address) << ": 0x"
                 << llvm::format("%02" PRIx64, val) << "\n";
  data_memory[address] = val;
}
