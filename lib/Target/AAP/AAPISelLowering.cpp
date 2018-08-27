//===-- AAPISelLowering.cpp - AAP DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AAPTargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "AAPISelLowering.h"
#include "AAPMachineFunctionInfo.h"
#include "AAPRegisterInfo.h"
#include "AAPSubtarget.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define DEBUG_TYPE "aap-lower"

using namespace llvm;

AAPTargetLowering::AAPTargetLowering(const TargetMachine &TM,
                                     const AAPSubtarget &STI)
    : TargetLowering(TM), STI(STI) {
  addRegisterClass(MVT::i16, &AAP::GR8RegClass);
  addRegisterClass(MVT::i16, &AAP::GR64RegClass);

  computeRegisterProperties(STI.getRegisterInfo());
  setStackPointerRegisterToSaveRestore(AAPRegisterInfo::getStackPtrRegister());
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);


  setMinFunctionAlignment(1);
  setPrefFunctionAlignment(2);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);

  // Custom DAGCombine
  setTargetDAGCombine(ISD::ADD);

  // No support for jump tables
  setMinimumJumpTableEntries(INT_MAX);
}

const char *AAPTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default:
    return nullptr;
  case AAPISD::RET_FLAG:
    return "AAPISD::RET_FLAG";
  case AAPISD::CALL:
    return "AAPISD::CALL";
  case AAPISD::Wrapper:
    return "AAPISD::Wrapper";
  case AAPISD::SELECT_CC:
    return "AAPISD::SELECT_CC";
  }
}

//===----------------------------------------------------------------------===//
//                       Custom Lowering Implementation
//===----------------------------------------------------------------------===//

SDValue AAPTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Unimplemented operation");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  }
}

SDValue AAPTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  SDLoc Loc(Op);
  EVT Ty = getPointerTy(DAG.getDataLayout());
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = N->getGlobal();
  int64_t Offset = N->getOffset();

  SDValue Result = DAG.getTargetGlobalAddress(GV, Loc, Ty, Offset);
  return DAG.getNode(AAPISD::Wrapper, Loc, Ty, Result);
}

//===----------------------------------------------------------------------===//
//                      Custom DAG Combine Implementation
//===----------------------------------------------------------------------===//

SDValue AAPTargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  default:
    return SDValue();
  case ISD::ADD:
    return PerformADDCombine(N, DCI);
  }
}

SDValue AAPTargetLowering::PerformADDCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  SDValue RHS = N->getOperand(1);
  ConstantSDNode *Const = dyn_cast<ConstantSDNode>(RHS);
  if (!Const)
    return SDValue();

  int64_t Value = Const->getSExtValue();
  if (Value >= 0)
    return SDValue();

  SelectionDAG &DAG = DCI.DAG;

  // fold add -ve -> sub +ve
  SDLoc Loc(N);
  RHS = DAG.getConstant(-Value, Loc, RHS.getValueType());

  SDValue Res = DAG.getNode(ISD::SUB, Loc, N->getValueType(0), N->getOperand(0),
                            RHS);
  DAG.ReplaceAllUsesWith(N, Res.getNode());
  return SDValue(N, 0);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "AAPGenCallingConv.inc"

SDValue AAPTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CC, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &Loc,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CC) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  AAPMachineFunctionInfo *MFuncInfo = MF.getInfo<AAPMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CC, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_AAP);

  // Create frame index for the start of the first vararg value
  if (IsVarArg) {
    unsigned Offset = CCInfo.getNextStackOffset();
    MFuncInfo->setVarArgsFrameIndex(MFI.CreateFixedObject(1, Offset, true));
  }

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) {
      default: {
#ifndef NDEBUG
        errs() << "LowerFormalArguments Unhandled argument type: "
               << RegVT.getEVTString() << "\n";
#endif
        llvm_unreachable(0);
      }
      case MVT::i16:
        unsigned VReg = RegInfo.createVirtualRegister(&AAP::GR64RegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, Loc, VReg, RegVT);

        // If this is an 8-bit value, it is really passed promoted to 16
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, Loc, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, Loc, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, Loc, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);
      }
    } else {
      // Sanity check
      assert(VA.isMemLoc());

      SDValue InVal;
      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) {
        int FI = MFI.CreateFixedObject(Flags.getByValSize(),
                                       VA.getLocMemOffset(), true);
        InVal = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      } else {
        // Load the argument to a virtual register
        unsigned ObjSize = VA.getLocVT().getSizeInBits() / 8;
        if (ObjSize > 2) {
          errs() << "LowerFormalArguments Unhandled argument type: "
                 << EVT(VA.getLocVT()).getEVTString() << "\n";
        }
        // Create the frame index object for this incoming parameter...
        int FI = MFI.CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

        // Create the SelectionDAG nodes corresponding to a load
        // from this parameter
        SDValue FIN = DAG.getFrameIndex(FI, MVT::i16);
        InVal = DAG.getLoad(VA.getLocVT(), Loc, Chain, FIN,
                            MachinePointerInfo::getFixedStack(MF, FI));
      }

      InVals.push_back(InVal);
    }
  }

  return Chain;
}


bool AAPTargetLowering::CanLowerReturn(
    CallingConv::ID CC, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Ctx) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CC, IsVarArg, MF, RVLocs, Ctx);

  // Check return values via the calling convention.
  return CCInfo.CheckReturn(Outs, RetCC_AAP);
}

SDValue AAPTargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CC, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &Loc,
    SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CC, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_AAP);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Add the link register as the first operand
  RetOps.push_back(
      DAG.getRegister(AAPRegisterInfo::getLinkRegister(), MVT::i16));

  // Add return registers to the CalleeSaveDisableRegs list.
  MachineRegisterInfo &MRI = DAG.getMachineFunction().getRegInfo();
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    MRI.disableCalleeSavedRegister(RVLocs[i].getLocReg());
  }

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, Loc, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(AAPISD::RET_FLAG, Loc, {MVT::Other, MVT::i16}, RetOps);
}
