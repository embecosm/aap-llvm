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
#include "AAP.h"
#include "AAPMachineFunctionInfo.h"
#include "AAPTargetMachine.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
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

AAPCC::CondCode AAPInstrInfo::reverseCondCode(AAPCC::CondCode CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Invalid condition code");
  case AAPCC::COND_EQ:
    return AAPCC::COND_NE;
  case AAPCC::COND_NE:
    return AAPCC::COND_EQ;
  case AAPCC::COND_LES:
  case AAPCC::COND_LEU:
  case AAPCC::COND_LTS:
  case AAPCC::COND_LTU:
    return AAPCC::COND_INVALID;
  }
}

unsigned AAPInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  switch (MI.getOpcode()) {
  default:
    return 0;
  case AAP::STB:
  case AAP::STW:
  case AAP::STB_postinc:
  case AAP::STW_postinc:
  case AAP::STB_predec:
  case AAP::STW_predec:
    break;
  }

  if (MI.getOperand(0).isFI() && MI.getOperand(1).isImm() &&
      MI.getOperand(1).getImm() == 0) {
    FrameIndex = MI.getOperand(0).getIndex();
    return MI.getOperand(2).getReg();
  }

  return 0;
}

unsigned AAPInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  default:
    return 0;
  case AAP::LDB:
  case AAP::LDW:
  case AAP::LDB_postinc:
  case AAP::LDW_postinc:
  case AAP::LDB_predec:
  case AAP::LDW_predec:
    break;
  }

  if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
      MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }

  return 0;
}

bool AAPInstrInfo::isBranchOffsetInRange(unsigned BranchOpc,
                                         int64_t BrOffset) const {
  switch (BranchOpc) {
  default:
    llvm_unreachable("Invalid opcode");
  case AAP::BRA:
    return AAP::isOff22(BrOffset);
  case AAP::BEQ_:
  case AAP::BNE_:
  case AAP::BLTS_:
  case AAP::BLES_:
  case AAP::BLTU_:
  case AAP::BLEU_:
    return AAP::isOff10(BrOffset);
  }
}

MachineBasicBlock *
AAPInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  assert(MI.getDesc().isBranch() && "Invalid opcode");

  // The branch target is always the first operand.
  return MI.getOperand(0).getMBB();
}

bool AAPInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  // Setup default return state
  TBB = FBB = nullptr;
  Cond.clear();

  MachineBasicBlock::iterator I = MBB.end();
  MachineBasicBlock::iterator FirstBr = MBB.end();
  MachineBasicBlock::iterator FirstUnCondBr = MBB.end();
  unsigned NumTerminators = 0;

  // Find the first branch and the first unconditional branch
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // If we see a non-terminator, we're done
    if (!isUnpredicatedTerminator(*I))
      break;

    ++NumTerminators;
    FirstBr = I;
    if (I->getDesc().isUnconditionalBranch() || I->getDesc().isIndirectBranch())
      FirstUnCondBr = I;
  }

  bool HaveUnCondBr = FirstUnCondBr != MBB.end();

  // Nuke everything after the first unconditional branch
  if (AllowModify && HaveUnCondBr) {
    while (std::next(FirstUnCondBr) != MBB.end()) {
      std::next(FirstUnCondBr)->eraseFromParent();
      --NumTerminators;
    }
  }

  // If the block ends in an unconditional branch we can't analyze it further
  if (HaveUnCondBr && FirstUnCondBr->getDesc().isIndirectBranch())
    return true;

  // Nuke the unconditional branch if it's actually a fallthrough
  if (AllowModify && HaveUnCondBr &&
      MBB.isLayoutSuccessor(getBranchDestBlock(*FirstUnCondBr))) {
    FirstUnCondBr->eraseFromParent();
    --NumTerminators;
    HaveUnCondBr = false;
  }

  // If the block ends in more than one conditional branch, we can't analyze it
  // TODO: Relax this constraint. We should be able to handle things such as:
  //   beq  L1, r0, r1
  //   bne  L2, r0, r1
  // More elegantly
  if ((!HaveUnCondBr && NumTerminators > 1) || NumTerminators > 2)
    return true;

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
  if (AllowModify && HaveUnCondBr && NumTerminators == 2 &&
      MBB.isLayoutSuccessor(getBranchDestBlock(*FirstBr))) {
    AAPCC::CondCode CC = getCondFromBranchOpcode(FirstBr->getOpcode());
    AAPCC::CondCode RCC = reverseCondCode(CC);
    if (RCC != AAPCC::COND_INVALID) {

      MachineBasicBlock *Target = getBranchDestBlock(*FirstUnCondBr);

      DebugLoc DL = MBB.findDebugLoc(FirstBr);

      MachineInstr *OldCondBr = &*FirstBr;
      FirstBr = BuildMI(&MBB, DL, get(getBranchOpcodeFromCond(RCC)))
                    .addMBB(Target)
                    .addReg(FirstBr->getOperand(1).getReg())
                    .addReg(FirstBr->getOperand(2).getReg());

      // Replace the conditional branch with the inverted branch
      FirstUnCondBr->eraseFromParent();
      OldCondBr->eraseFromParent();
      --NumTerminators;
      HaveUnCondBr = false;
    }
  }

  // If the block has no terminators, it's just a fallthrough
  if (NumTerminators == 0)
    return false;

  // Only an unconditional branch, set TBB to the destination block
  if (HaveUnCondBr && NumTerminators == 1) {
    TBB = getBranchDestBlock(*FirstUnCondBr);
    return false;
  }

  // Conditional branch /w fallthrough. Set TBB to destination and
  // set Cond to operands to evaluate the condition
  if (!HaveUnCondBr && NumTerminators == 1) {
    TBB = getBranchDestBlock(*FirstBr);
    AAPCC::CondCode CC = getCondFromBranchOpcode(FirstBr->getOpcode());
    Cond.push_back(MachineOperand::CreateImm(CC));
    Cond.push_back(FirstBr->getOperand(1));
    Cond.push_back(FirstBr->getOperand(2));
    return false;
  }

  // Unconditional branch preceded by conditional branch
  assert(HaveUnCondBr && NumTerminators == 2);

  TBB = getBranchDestBlock(*FirstBr);
  FBB = getBranchDestBlock(*FirstUnCondBr);

  AAPCC::CondCode CC = getCondFromBranchOpcode(FirstBr->getOpcode());
  Cond.push_back(MachineOperand::CreateImm(CC));
  Cond.push_back(FirstBr->getOperand(1));
  Cond.push_back(FirstBr->getOperand(2));
  return false;
}

unsigned AAPInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL, int *BytesAdded) const {
  assert(TBB && "InsertBranch cannot insert a fallthrough");
  assert(Cond.size() == 3 || Cond.size() == 0);

  MachineInstr *MI;

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch cannot have multiple successors");
    MI = BuildMI(&MBB, DL, get(AAP::BRA)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(*MI);
    return 1;
  }
  // Conditional branch
  unsigned Count = 0;
  AAPCC::CondCode CC = (AAPCC::CondCode)Cond[0].getImm();
  MI = BuildMI(&MBB, DL, get(getBranchOpcodeFromCond(CC)))
           .addMBB(TBB)
           .addReg(Cond[1].getReg())
           .addReg(Cond[2].getReg());
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(*MI);
  ++Count;

  if (FBB) {
    MI = BuildMI(&MBB, DL, get(AAP::BRA)).addMBB(FBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(*MI);
    ++Count;
  }
  return Count;
}

unsigned AAPInstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                            MachineBasicBlock &DestBB,
                                            const DebugLoc &DL,
                                            int64_t BrOffset,
                                            RegScavenger *RS) const {
  // Since AAP does not have an indirect branch, the next best thing is to use
  // the direct 22-immediate unconditional branch instead. This function is used
  // by the BranchRelaxation pass in order to generate a long unconditional
  // branch as an alternative to a long conditional branch, meaning the effect
  // of using a direct branch with a larger immediate value gives the same
  // benefit as an indirect branch of drastically increasing the range.
  auto &MI = *BuildMI(&MBB, DL, get(AAP::BRA)).addMBB(&DestBB);

  return getInstSizeInBytes(MI);
}

unsigned AAPInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
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
    // Add to the number of bytes removed
    if (BytesRemoved)
      *BytesRemoved += getInstSizeInBytes(*I);
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
  assert(Cond.size() == 3 && "Invalid branch");

  AAPCC::CondCode RCC = reverseCondCode((AAPCC::CondCode)Cond[0].getImm());
  if (RCC == AAPCC::COND_INVALID)
    return true;

  Cond[0].setImm(RCC);
  return false;
}

unsigned AAPInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    return MI.getDesc().getSize();
  case TargetOpcode::EH_LABEL:
  case TargetOpcode::IMPLICIT_DEF:
  case TargetOpcode::KILL:
  case TargetOpcode::DBG_VALUE:
    return 0;
  case TargetOpcode::INLINEASM: {
    const MachineFunction &MF = *MI.getParent()->getParent();
    const auto &TM = static_cast<const AAPTargetMachine &>(MF.getTarget());
    return getInlineAsmLength(MI.getOperand(0).getSymbolName(),
                              *TM.getMCAsmInfo());
  }
  }
}
