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
}

AAPSimState::~AAPSimState() {
  delete code_memory;
  delete data_memory;
}

uint16_t AAPSimState::getReg(int reg) const {
  assert (reg < 64 && "Invalid register");
  return base_regs[reg];
}

void AAPSimState::setReg(int reg, uint16_t val) {
  assert (reg < 64 && "Invalid register");
  base_regs[reg] = val;
}

uint32_t AAPSimState::getPC() const {
  return pc_w;
}

void AAPSimState::setPC(uint32_t val_w) {
  assert (val_w < 0xffffff && "Invalid PC");
  pc_w = val_w;
}

uint8_t AAPSimState::getCodeMem(uint32_t address) const {
  assert (address < 0x1ffffff && "Invalid address");
  return code_memory[address];
}

void AAPSimState::setCodeMem(uint32_t address, uint8_t val) {
  assert (address < 0x1ffffff && "Invalid address");
  code_memory[address] = val;
}

uint8_t AAPSimState::getDataMem(uint32_t address) const {
  assert (address < 0xffff && "Invalid address");
  return data_memory[address];
}

void AAPSimState::setDataMem(uint32_t address, uint8_t val) {
  assert (address < 0xffff && "Invalid address");
  data_memory[address] = val;
}
