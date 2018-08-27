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

#ifndef LLVM_LIB_TARGET_AAP_AAPINSTRINFO_H
#define LLVM_LIB_TARGET_AAP_AAPINSTRINFO_H

#include "AAPRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "AAPGenInstrInfo.inc"

namespace llvm {
namespace AAPCC {
// AAP specific condition codes
enum CondCode {
  COND_EQ = 0,
  COND_NE = 1,
  COND_LTS = 2,
  COND_LES = 3,
  COND_LTU = 4,
  COND_LEU = 5,
  COND_INVALID = -1
};
}

class AAPInstrInfo : public AAPGenInstrInfo {
public:
  AAPInstrInfo();

  unsigned isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  unsigned isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
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

  static AAPCC::CondCode getCondFromBranchOpcode(unsigned Opcode);
  static unsigned getBranchOpcodeFromCond(AAPCC::CondCode CC);
  static AAPCC::CondCode reverseCondCode(AAPCC::CondCode CC);
};
} // namespace llvm

#endif
