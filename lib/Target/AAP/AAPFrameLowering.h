//===-- AAPFrameLowering.h - Frame info for AAP Target ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_AAPFRAMELOWERING_H
#define LLVM_LIB_TARGET_AAP_AAPFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class AAPSubtarget;

class AAPFrameLowering : public TargetFrameLowering {
public:
  AAPFrameLowering()
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                            /*StackAlignment=*/2, /*LocalAreaOffset=*/0,
                            /*TransientStackAlignment=*/2) {}

  bool hasFP(const MachineFunction &MF) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  const SpillSlot *
  getCalleeSavedSpillSlots(unsigned &NumEntries) const override;

  int getFrameIndexReference(const MachineFunction &MF, int FI,
                             unsigned &FrameReg) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
};
} // namespace llvm

#endif
