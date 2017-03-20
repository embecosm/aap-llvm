//===-- AAPInstrInfo.cpp - AAP Instruction Information ----------------===//
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

#include "AAPInstrInfo.h"
#include "AAPMachineFunctionInfo.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "AAPGenInstrInfo.inc"

// Pin the vtable to this file.
void AAPInstrInfo::anchor() {}

AAPInstrInfo::AAPInstrInfo(AAPSubtarget &STI)
    : AAPGenInstrInfo(AAP::ADJCALLSTACKDOWN, AAP::ADJCALLSTACKUP), TRI() {}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//

AAPCC::CondCode AAPInstrInfo::getCondFromBranchOpcode(unsigned Opcode) const {
  switch (Opcode) {
  default:
    return AAPCC::COND_INVALID;
  case AAP::BEQ_:
    return AAPCC::COND_EQ;
  case AAP::BNE_:
    return AAPCC::COND_NE;
  case AAP::BLTS_:
    return AAPCC::COND_LTS;
  case AAP::BLES_:
    return AAPCC::COND_LES;
  case AAP::BLTU_:
    return AAPCC::COND_LTU;
  case AAP::BLEU_:
    return AAPCC::COND_LEU;
  }
}

unsigned AAPInstrInfo::getBranchOpcodeFromCond(AAPCC::CondCode CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Invalid condition code");
  case AAPCC::COND_EQ:
    return AAP::BEQ_;
  case AAPCC::COND_NE:
    return AAP::BNE_;
  case AAPCC::COND_LTS:
    return AAP::BLTS_;
  case AAPCC::COND_LES:
    return AAP::BLES_;
  case AAPCC::COND_LTU:
    return AAP::BLTU_;
  case AAPCC::COND_LEU:
    return AAP::BLEU_;
  }
}

bool AAPInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  // Find the first unconditional branch
  MachineBasicBlock::iterator I = MBB.end();
  MachineBasicBlock::iterator CondBrIter = MBB.end();
  MachineBasicBlock::iterator UnCondBrIter = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // If we see a non-terminator, we're done
    if (!isUnpredicatedTerminator(*I))
      break;
    // Unconditional branch opcode
    if (I->getOpcode() == AAP::BRA)
      UnCondBrIter = I;
  }

  // Find either:
  // - The first conditional branch before an unconditional branch
  // - The first conditional branch, if no unconditional branch was found
  if (UnCondBrIter == MBB.end())
    I = MBB.end();
  else
    I = UnCondBrIter;
  unsigned NumCondBr = 0;
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // If we see a non-terminator, we're done
    if (!isUnpredicatedTerminator(*I))
      break;

    // Conditional branch opcode
    switch (I->getOpcode()) {
    default:
      break;
    case AAP::BEQ_:
    case AAP::BNE_:
    case AAP::BLTS_:
    case AAP::BLES_:
    case AAP::BLTU_:
    case AAP::BLEU_:
      ++NumCondBr;
      CondBrIter = I;
    }
  }

  // If there's more than one conditional branch, we can't analyze it
  // TODO: Relax this constraint. We should be able to handle things such as:
  //   beq  L1, r0, r1
  //   bne  L2, r0, r1
  // More elegantly
  if (NumCondBr > 1)
    return true;

  bool HaveCondBr = CondBrIter != MBB.end();
  bool HaveUnCondBr = UnCondBrIter != MBB.end();

  if (AllowModify && HaveUnCondBr) {
    // Nuke everything after the first unconditional branch
    while (std::next(UnCondBrIter) != MBB.end())
      std::next(UnCondBrIter)->eraseFromParent();

    // Remove the unconditional branch if it's actually a fallthrough
    if (MBB.isLayoutSuccessor(UnCondBrIter->getOperand(0).getMBB())) {
      UnCondBrIter->eraseFromParent();
      HaveUnCondBr = false;
    }

    if (HaveCondBr && HaveUnCondBr) {
      // We may be able to modify the following code:
      //
      //   bCC L1
      //   bra L2
      // L1:
      //   ...
      // L2:
      //
      // To something more efficient such as
      //
      //   bnCC L2
      // L1:
      //   ...
      // L2:
      //
      if (MBB.isLayoutSuccessor(CondBrIter->getOperand(0).getMBB())) {
        unsigned Opcode = CondBrIter->getOpcode();
        if (Opcode == AAP::BEQ_ || Opcode == AAP::BNE_) {
          unsigned InvertedOpcode = Opcode == AAP::BEQ_ ? AAP::BNE_ : AAP::BEQ_;

          MachineBasicBlock *Target = UnCondBrIter->getOperand(0).getMBB();
          DebugLoc DL = MBB.findDebugLoc(CondBrIter);

          MachineInstr *OldCondBr = &*CondBrIter;
          MachineInstrBuilder MIB;
          CondBrIter = BuildMI(&MBB, DL, get(InvertedOpcode))
              .addMBB(Target)
              .addReg(CondBrIter->getOperand(1).getReg())
              .addReg(CondBrIter->getOperand(2).getReg());

          // Replace the conditional branch with the inverted branch
          UnCondBrIter->eraseFromParent();
          OldCondBr->eraseFromParent();
          HaveUnCondBr = false;
        }
      }
    }
  }

  if (!HaveUnCondBr && !HaveCondBr) {
    // Block ends with no branches (falls through to its successor)
    TBB = FBB = nullptr;
  } else if (HaveUnCondBr && HaveCondBr) {
    // Unconditional branch preceded by conditional branch
    TBB = CondBrIter->getOperand(0).getMBB();
    FBB = UnCondBrIter->getOperand(0).getMBB();

    AAPCC::CondCode CC = getCondFromBranchOpcode(CondBrIter->getOpcode());
    Cond.push_back(MachineOperand::CreateImm(CC));
    Cond.push_back(CondBrIter->getOperand(1));
    Cond.push_back(CondBrIter->getOperand(2));
  } else if (HaveUnCondBr && !HaveCondBr) {
    // Only an unconditional branch, set TBB to the destination block
    TBB = UnCondBrIter->getOperand(0).getMBB();
  } else {
    assert(!HaveUnCondBr && HaveCondBr);
    // Conditional branch /w fallthrough. Set TBB to destination and
    // set Cond to operands to evaluate the condition
    TBB = CondBrIter->getOperand(0).getMBB();
    AAPCC::CondCode CC = getCondFromBranchOpcode(CondBrIter->getOpcode());
    Cond.push_back(MachineOperand::CreateImm(CC));
    Cond.push_back(CondBrIter->getOperand(1));
    Cond.push_back(CondBrIter->getOperand(2));
  }
  return false;
}

unsigned AAPInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL,
                                    int *BytesAdded) const {
  assert(TBB && "InsertBranch cannot insert a fallthrough");
  assert(Cond.size() == 3 || Cond.size() == 0);
  assert(!BytesAdded && "Code size not handled");

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch cannot have multiple successors");
    BuildMI(&MBB, DL, get(AAP::BRA)).addMBB(TBB);
    return 1;
  }
  // Conditional branch
  unsigned Count = 0;
  AAPCC::CondCode CC = (AAPCC::CondCode)Cond[0].getImm();
  BuildMI(&MBB, DL, get(getBranchOpcodeFromCond(CC)))
      .addMBB(TBB)
      .addReg(Cond[1].getReg())
      .addReg(Cond[2].getReg());
  ++Count;

  if (FBB) {
    BuildMI(&MBB, DL, get(AAP::BRA)).addMBB(FBB);
    ++Count;
  }
  return Count;
}

unsigned AAPInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
  assert(!BytesRemoved && "Code size not handled");

  unsigned Count = 0;
  auto I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    if (!I->isBranch())
      break;
    // Remove the branch
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }
  return Count;
}

void AAPInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, unsigned DestReg,
                               unsigned SrcReg, bool KillSrc) const {
  assert(AAP::GR64RegClass.contains(DestReg) &&
         AAP::GR64RegClass.contains(DestReg) &&
         "Impossible register-register copy");

  // FIXME: If possible with short insn, build that instead
  BuildMI(MBB, I, DL, get(AAP::MOV_r), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

void AAPInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       unsigned SrcReg, bool isKill,
                                       int FrameIdx,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIdx),
      MachineMemOperand::MOStore, MFrameInfo.getObjectSize(FrameIdx),
      MFrameInfo.getObjectAlignment(FrameIdx));

  assert((RC == &AAP::GR8RegClass || RC == &AAP::GR64RegClass) &&
         "Unknown register class to store to stack slot");

  BuildMI(MBB, MI, DL, get(AAP::STW))
      .addFrameIndex(FrameIdx)
      .addImm(0)
      .addReg(SrcReg, getKillRegState(isKill))
      .addMemOperand(MMO);
}

void AAPInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        unsigned DstReg, int FrameIdx,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFrameInfo = MF.getFrameInfo();

  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIdx),
      MachineMemOperand::MOLoad, MFrameInfo.getObjectSize(FrameIdx),
      MFrameInfo.getObjectAlignment(FrameIdx));

  assert((RC == &AAP::GR8RegClass || RC == &AAP::GR64RegClass) &&
         "Unknown register class to store to stack slot");

  BuildMI(MBB, MI, DL, get(AAP::LDW), DstReg)
      .addFrameIndex(FrameIdx)
      .addImm(0)
      .addMemOperand(MMO);
}

/// ReverseBranchCondition - Return the inverse opcode of the
/// specified Branch instruction.
bool AAPInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  return false;
}
