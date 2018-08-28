//===-- AAPRegisterInfo.h - AAP Register Information Impl -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_AAPREGISTERINFO_H
#define LLVM_LIB_TARGET_AAP_AAPREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "AAPGenRegisterInfo.inc"

namespace llvm {
class AAPRegisterInfo : public AAPGenRegisterInfo {
public:
  AAPRegisterInfo();

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  const MCPhysReg *
  getCalleeSavedRegs(const MachineFunction *MF = nullptr) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override {
    return true;
  }

  static void adjustReg(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI, const DebugLoc &DL,
                        unsigned DestReg, unsigned SrcReg, int64_t Val,
                        MachineInstr::MIFlag Flag);

  static unsigned getLinkRegister();
  static unsigned getStackPtrRegister();
  static unsigned getFramePtrRegister();
};
}

#endif
