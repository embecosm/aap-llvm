//===-- AAPISelDAGToDAG.cpp - A dag to dag inst selector for AAP ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the AAP target.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "AAPISelLowering.h"
#include "AAPTargetMachine.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"

#define DEBUG_TYPE "aap-isel"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AAPDAGToDAGISel - AAP specific code to select AAP machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//

namespace {
class AAPISelDAGToDAG : public SelectionDAGISel {
public:
  AAPISelDAGToDAG(AAPTargetMachine &TM) : SelectionDAGISel(TM) {}

  StringRef getPassName() const override {
    return "AAP DAG->DAG Pattern Instruction Selection";
  }

  void Select(SDNode *N) override;

  bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintID,
                                    std::vector<SDValue> &OutOps) override;

#include "AAPGenDAGISel.inc"
private:
  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool SelectAddr_MO3(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool SelectAddr_MO10(SDValue Addr, SDValue &Base, SDValue &Offset);
};
}

void AAPISelDAGToDAG::Select(SDNode *Node) {
  // Dump information about the Node being selected
  LLVM_DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  // Instruction Selection not handled by the auto-generated
  // tablegen selection should be handled here.
  switch (Node->getOpcode()) {
  default:
    break;
  case ISD::FrameIndex: {
    assert(Node->getValueType(0) == MVT::i16);

    int FI = cast<FrameIndexSDNode>(Node)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i16);
    SDLoc Loc(Node);

    // Handle single use
    SDNode *N = CurDAG->getMachineNode(AAP::LEA, Loc, MVT::i16, TFI,
        CurDAG->getTargetConstant(0, Loc, MVT::i16));
    ReplaceNode(Node, N);
    return;
  }
  }

  // Select the default instruction
  SelectCode(Node);
}

bool AAPISelDAGToDAG::SelectInlineAsmMemoryOperand(
    const SDValue &Op, unsigned ConstraintID, std::vector<SDValue> &OutOps) {
  switch (ConstraintID) {
  default:
    return true;
  case InlineAsm::Constraint_i:
  case InlineAsm::Constraint_m:
    SDLoc Loc(Op);
    SDValue RC =
        CurDAG->getTargetConstant(AAP::GR64RegClass.getID(), Loc, MVT::i16);
    SDNode *N = CurDAG->getMachineNode(TargetOpcode::COPY_TO_REGCLASS, Loc,
                                       Op.getValueType(), Op, RC);
    SDValue Zero = CurDAG->getTargetConstant(0, Loc, MVT::i16);
    OutOps.push_back(SDValue(N, 0));
    OutOps.push_back(Zero);
    break;
  }
  return false;
}

bool AAPISelDAGToDAG::SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset) {
  // if Address is FI, get the TargetFrameIndex
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    SDLoc Loc(FIN);
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i16);
    Offset = CurDAG->getTargetConstant(0, Loc, MVT::i16);
    return true;
  }

  if ((Addr.getOpcode() == ISD::TargetExternalSymbol ||
       Addr.getOpcode() == ISD::TargetGlobalAddress)) {
    return false;
  }

  bool isConstantOffset = CurDAG->isBaseWithConstantOffset(Addr);
  bool isSubOffset = Addr.getOpcode() == ISD::SUB;

  if (!(isConstantOffset || isSubOffset))
    return false;

  // Addresses of the form Addr+const, Addr-const or Addr|const

  if (!isa<ConstantSDNode>(Addr.getOperand(1)))
    return false;

  ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1));
  if (!isInt<16>(CN->getSExtValue()))
    return false;

  SDLoc Loc(CN);
  // If the first operand is a FI, get the TargetFI Node
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr.getOperand(0)))
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i16);
  else
    Base = Addr.getOperand(0);

  int64_t off = CN->getSExtValue();
  if (isSubOffset)
    off = -off;

  Offset = CurDAG->getTargetConstant(off, Loc, MVT::i16);
  return true;
}

bool AAPISelDAGToDAG::SelectAddr_MO3(SDValue Addr, SDValue &Base,
                                     SDValue &Offset) {
  SDValue B, O;
  if (!SelectAddr(Addr, B, O))
    return false;

  int64_t c = dyn_cast<ConstantSDNode>(O)->getSExtValue();
  if (!isInt<3>(c))
    return false;

  Base = B;
  Offset = O;

  return true;
}

bool AAPISelDAGToDAG::SelectAddr_MO10(SDValue Addr, SDValue &Base,
                                      SDValue &Offset) {
  SDValue B, O;
  if (!SelectAddr(Addr, B, O))
    return false;

  int64_t c = dyn_cast<ConstantSDNode>(O)->getSExtValue();
  if (!isInt<10>(c))
    return false;

  Base = B;
  Offset = O;

  return true;
}

FunctionPass *llvm::createAAPISelDag(AAPTargetMachine &TM) {
  return new AAPISelDAGToDAG(TM);
}
