//===-- AAPInstrInfo.h - AAP Instruction Information ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AAP implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef AAPINSTRINFO_H
#define AAPINSTRINFO_H

#include "AAPRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "AAPGenInstrInfo.inc"

namespace llvm {

class AAPSubtarget;

class AAPInstrInfo : public AAPGenInstrInfo {
  const AAPRegisterInfo TRI;
  virtual void anchor();

public:
  AAPInstrInfo(AAPSubtarget &STI);

  const TargetRegisterInfo &getRegisterInfo() const { return TRI; }

  bool AnalyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        DebugLoc DL) const override;

  unsigned RemoveBranch(MachineBasicBlock &MBB) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   DebugLoc DL, unsigned DestReg, unsigned SrcReg,
                   bool KillSrc) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, unsigned SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, unsigned DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  bool
  ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;
};
}

#endif
