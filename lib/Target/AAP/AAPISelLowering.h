//===-- AAPISelLowering.h - AAP DAG Lowering Interface ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that AAP uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AAP_AAPISELLOWERING_H
#define LLVM_LIB_TARGET_AAP_AAPISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class AAPSubtarget;
class TargetMachine;
class SelectionDAG;

namespace AAPISD {
enum NodeType {
  // Start the numbering where the builtin ops and target ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,

  /// Return with a flag operand. Operand 0 is the chain.
  RET_FLAG,

  /// CALL - A node to wrap calls.
  CALL,

  /// Wrapper - A wrapper node for TargetConstantPool, TargetExternalSymbol,
  /// and TargetGlobalAddress.
  Wrapper,

  /// BR_CC - Custom brcc node, where the condition code is an AAP
  /// specific value
  BR_CC,

  /// SELECT_CC - Custom selectcc node, where the condition code is an
  /// AAP specific value
  SELECT_CC
};
}

class AAPTargetLowering : public TargetLowering {
  const AAPSubtarget &STI;
public:
  explicit AAPTargetLowering(const TargetMachine &TM, const AAPSubtarget &STI);

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *MBB) const override;

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  SDValue PerformADDCombine(SDNode *N, DAGCombinerInfo &DCI) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CC, bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &Loc, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  bool CanLowerReturn(CallingConv::ID CC, MachineFunction &MF, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Ctx) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CC, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &Loc,
                      SelectionDAG &DAG) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  MachineBasicBlock *EmitSELECT_CC(MachineInstr &MI,
                                   MachineBasicBlock *MBB) const;

  MachineBasicBlock *EmitBR_CC(MachineInstr &MI,
                               MachineBasicBlock *MBB) const;
};
}

#endif
