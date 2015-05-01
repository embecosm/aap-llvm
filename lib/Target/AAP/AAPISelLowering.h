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

#ifndef AAPISELLOWERING_H
#define AAPISELLOWERING_H

#include "AAP.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {

// Forward delcarations
class AAPSubtarget;
class AAPTargetMachine;

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

//===--------------------------------------------------------------------===//
// TargetLowering Implementation
//===--------------------------------------------------------------------===//
class AAPTargetLowering : public TargetLowering {
public:
  explicit AAPTargetLowering(const TargetMachine &TM, const AAPSubtarget &STI);

  /// LowerOperation - Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// ReplaceNodeResults - Replace the results of node with an illegal result
  /// type with new values built out of custom code.
  ///
  // void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue>&Results,
  //                        SelectionDAG &DAG) const override;

  /// getTargetNodeName - This method returns the name of a target specific
  //  DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;

  /// Custom lowering
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr *MI,
                              MachineBasicBlock *MBB) const override;

private:
  MachineBasicBlock *emitBrCC(MachineInstr *MI, MachineBasicBlock *MBB) const;
  MachineBasicBlock *emitSelectCC(MachineInstr *MI,
                                  MachineBasicBlock *MBB) const;

  SDValue LowerCCCCallTo(SDValue Chain, SDValue Callee,
                         CallingConv::ID CallConv, bool isVarArg,
                         bool isTailCall,
                         const SmallVectorImpl<ISD::OutputArg> &Outs,
                         const SmallVectorImpl<SDValue> &OutVals,
                         const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl,
                         SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerCCCArguments(SDValue Chain, CallingConv::ID CallConv,
                            bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl,
                            SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl,
                          SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               SDLoc dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, SDLoc dl,
                      SelectionDAG &DAG) const override;
};
}

#endif
