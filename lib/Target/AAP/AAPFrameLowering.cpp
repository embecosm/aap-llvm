//===-- AAPFrameLowering.cpp - Frame info for AAP Target ------------------===//
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

#include "AAPFrameLowering.h"
#include "AAPInstrInfo.h"
#include "AAPMachineFunctionInfo.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

bool AAPFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MF.getSubtarget().getRegisterInfo()->needsStackRealignment(MF) ||
         MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

void AAPFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Unconditionally spill RA and FP only if the function uses a frame
  // pointer.
  if (hasFP(MF)) {
    SavedRegs.set(AAP::R0);
    SavedRegs.set(AAPRegisterInfo::getFramePtrRegister());
  }
}

const TargetFrameLowering::SpillSlot *
AAPFrameLowering::getCalleeSavedSpillSlots(unsigned &NumEntries) const {
  // Ensure that the frame pointer is stored at a constant stack offset.
  NumEntries = 2;
  static const SpillSlot Offsets[] = {
      {AAP::R0, -2}, {AAPRegisterInfo::getFramePtrRegister(), -4}};
  return Offsets;
}

int AAPFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                             unsigned &FrameReg) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  // Determine the offset to the start of the frame.
  int Offset = MFrameInfo.getObjectOffset(FI) - getOffsetOfLocalArea() +
               MFrameInfo.getOffsetAdjustment();

  // Determine whether to use the frame pointer. The logic is kept as a set of
  // if statements for clarity.
  bool UseFP = false;
  if (hasFP(MF)) {
    // For callee saved registers the stack pointer must be used.

    const std::vector<CalleeSavedInfo> &CSI = MFrameInfo.getCalleeSavedInfo();
    int MinCSFI = 0;
    int MaxCSFI = -1;
    for (auto &CS : CSI) {
      int CSFI = CS.getFrameIdx();
      if (CSFI < MinCSFI)
        MinCSFI = CSFI;
      if (CSFI > MaxCSFI)
        MaxCSFI = CSFI;
    }

    // Not a callee saved register:
    if (FI < MinCSFI || FI > MaxCSFI) {

      // If the offset is to a fixed stack object, the frame pointer should be
      // used.
      if (MFrameInfo.isFixedObjectIndex(FI))
        UseFP = true;

      // If the stack frame contains variable sized objects, the frame pointer
      // should be used.
      if (MFrameInfo.hasVarSizedObjects())
        UseFP = true;

      // For other objects the frame pointer should only be used if the offset
      // is within a safe range.
      if (Offset >= -512)
        UseFP = true;
    }
  }

  if (!UseFP) {
    FrameReg = AAPRegisterInfo::getStackPtrRegister();
    Offset += MFrameInfo.getStackSize();
  } else
    FrameReg = AAPRegisterInfo::getFramePtrRegister();

  // NOTE: If AAP did not pass all varargs on the stack, an offset to the frame
  // pointer would need to take into account the size of the varargs stored off
  // the stack to remain accurate.

  return Offset;
}

// Ensure that for varargs, the size of the caller reserved frame is taken into
// account when calculating stack offset.
bool AAPFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator AAPFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  unsigned SP = AAPRegisterInfo::getStackPtrRegister();
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  if (MF.getFrameInfo().hasVarSizedObjects()) {
    // If the current frame contains variable sized stack objects, IE created by
    // alloca, the stack pointer must be adjusted.
    int64_t NumBytes = MI->getOperand(0).getImm();

    if (NumBytes != 0) {
      NumBytes = alignSPAdjust(NumBytes);

      if (MI->getOpcode() == AAP::ADJCALLSTACKDOWN)
        NumBytes = -NumBytes;

      AAPRegisterInfo::adjustReg(MBB, MI, DL, SP, SP, NumBytes,
                                 MachineInstr::NoFlags);
    }
  }
  return MBB.erase(MI);
}

void AAPFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  // Get the number of bytes to allocate from the FrameInfo.
  const uint64_t StackSize = MFrameInfo.getStackSize();

  // Adjust the stack pointer down by the number of bytes needed.
  AAPRegisterInfo::adjustReg(MBB, MBBI, DL, SP, SP, -StackSize,
                             MachineInstr::FrameSetup);

  if (!hasFP(MF))
    return;

  unsigned FP = AAPRegisterInfo::getFramePtrRegister();

  // To ensure that the previous state of the frame pointer remains available to
  // be restored, the frame pointer adjustment must be done after the callee
  // saved registers are spilled to the stack, since the frame pointer is a
  // callee saved register.
  std::advance(MBBI, MFrameInfo.getCalleeSavedInfo().size());

  // NOTE: If AAP did not pass all varargs on the stack, the restore adjustment
  // would need to take into account the size of the varargs stored off the
  // stack to remain accurate.
  AAPRegisterInfo::adjustReg(MBB, MBBI, DL, FP, SP, StackSize,
                             MachineInstr::FrameSetup);
}

void AAPFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  assert((MBBI->getDesc().isReturn()) &&
         "Epilogue can only be inserted in returning blocks");
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned SP = AAPRegisterInfo::getStackPtrRegister();

  // Get the number of bytes to deallocate from the FrameInfo
  const uint64_t StackSize = MFrameInfo.getStackSize();

  if (MFrameInfo.hasVarSizedObjects() ||
      MF.getSubtarget().getRegisterInfo()->needsStackRealignment(MF)) {
    unsigned FP = AAPRegisterInfo::getFramePtrRegister();

    // To ensure that the state of the stack pointer can be restored to the
    // current state of the frame pointer, the adjustment must be done before
    // the callee saved registers are restored from the stack, since the frame
    // pointer is a callee saved register.
    MachineBasicBlock::iterator PreRestore = MBBI;
    std::advance(PreRestore, -MFrameInfo.getCalleeSavedInfo().size());

    // NOTE: If AAP did not pass all varargs on the stack, the restore
    // adjustment would need to take into account the size of the varargs stored
    // off the stack to remain accurate.
    AAPRegisterInfo::adjustReg(MBB, PreRestore, DL, SP, FP, -StackSize,
                               MachineInstr::FrameDestroy);
  }

  // Adjust the stack pointer up by the number of bytes needed.
  AAPRegisterInfo::adjustReg(MBB, MBBI, DL, SP, SP, StackSize,
                             MachineInstr::FrameDestroy);
}
