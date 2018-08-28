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
#include "AAPInstrInfo.h"
#include "AAPRegisterInfo.h"
#include "AAPSubtarget.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
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

  // Expand all i1 -> {i8, i16} loads
  setLoadExtAction(ISD::EXTLOAD, MVT::i8, MVT::i1, Promote);
  setLoadExtAction(ISD::EXTLOAD, MVT::i16, MVT::i1, Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8, MVT::i1, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, MVT::i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, MVT::i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i1, Expand);

  // Expand i8 -> i16 loads
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, MVT::i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i8, Expand);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i16, Custom);

  // Use SHL & SRA to sign extend in register
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  // Division operations unsupported by the architecture
  setOperationAction(ISD::SDIV, MVT::i16, Expand);
  setOperationAction(ISD::UDIV, MVT::i16, Expand);
  setOperationAction(ISD::SREM, MVT::i16, Expand);
  setOperationAction(ISD::UREM, MVT::i16, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i16, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i16, Expand);

  // Multiplication operations unsupported by the architecture
  setOperationAction(ISD::MUL, MVT::i16, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i16, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i16, Expand);
  setOperationAction(ISD::MULHS, MVT::i16, Expand);
  setOperationAction(ISD::MULHU, MVT::i16, Expand);
  setOperationAction(ISD::SMULO, MVT::i16, Expand);
  setOperationAction(ISD::UMULO, MVT::i16, Expand);

  // Adds and subs can produce carry
  setOperationAction(ISD::ADDC, MVT::i16, Legal);
  setOperationAction(ISD::SUBC, MVT::i16, Legal);

  // Use ADDE/SUBE
  setOperationAction(ISD::SADDO, MVT::i16, Expand);
  setOperationAction(ISD::UADDO, MVT::i16, Expand);
  setOperationAction(ISD::SSUBO, MVT::i16, Expand);
  setOperationAction(ISD::USUBO, MVT::i16, Expand);

  // Compound shifting operations unsupported by the architecture
  setOperationAction(ISD::SHL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::ROTL, MVT::i16, Expand);
  setOperationAction(ISD::ROTR, MVT::i16, Expand);

  // Bit manipulation operations unsupported by the architecture
  setOperationAction(ISD::BSWAP, MVT::i16, Expand);
  setOperationAction(ISD::CTLZ, MVT::i16, Expand);
  setOperationAction(ISD::CTTZ, MVT::i16, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i16, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i16, Expand);
  setOperationAction(ISD::CTPOP, MVT::i16, Expand);
  setOperationAction(ISD::BITREVERSE, MVT::i16, Expand);

  // Handle conditionals via brcc and selectcc
  setOperationAction(ISD::BRCOND, MVT::i16, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SELECT, MVT::i16, Expand);
  setOperationAction(ISD::SETCC, MVT::i16, Expand);
  setOperationAction(ISD::SETCC, MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i16, Custom);
  setOperationAction(ISD::BR_CC, MVT::i16, Custom);

  // Expand some condition codes which are not natively supported
  setCondCodeAction(ISD::SETGT, MVT::i16, Expand);
  setCondCodeAction(ISD::SETGE, MVT::i16, Expand);
  setCondCodeAction(ISD::SETUGT, MVT::i16, Expand);
  setCondCodeAction(ISD::SETUGE, MVT::i16, Expand);

  // BR_JT unsupported by the architecture
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

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
  case ISD::ExternalSymbol:
    return LowerExternalSymbol(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
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

SDValue AAPTargetLowering::LowerExternalSymbol(SDValue Op,
                                               SelectionDAG &DAG) const {
  EVT Ty = getPointerTy(DAG.getDataLayout());
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();

  SDValue Result = DAG.getTargetExternalSymbol(Sym, Ty);
  return DAG.getNode(AAPISD::Wrapper, SDLoc(Op), Ty, Result);
}

// Get the AAP specific condition code for a given CondCode DAG node.
static AAPCC::CondCode getAAPCondCode(ISD::CondCode CC) {
  switch (CC) {
  // These have a direct equivalent
  case ISD::SETEQ:
    return AAPCC::COND_EQ;
  case ISD::SETNE:
    return AAPCC::COND_NE;
  case ISD::SETLT:
    return AAPCC::COND_LTS;
  case ISD::SETLE:
    return AAPCC::COND_LES;
  case ISD::SETULT:
    return AAPCC::COND_LTU;
  case ISD::SETULE:
    return AAPCC::COND_LEU;
  // Other condition codes are unhandled
  default:
    llvm_unreachable("Unhandled condition code for branch lowering");
  }
}

SDValue AAPTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc Loc(Op);

  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueValue = Op.getOperand(2);
  SDValue FalseValue = Op.getOperand(3);

  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  // get equivalent AAP condition code
  AAPCC::CondCode TargetCC = getAAPCondCode(CC);

  SDValue Ops[] = {LHS, RHS, TrueValue, FalseValue,
                   DAG.getConstant(TargetCC, Loc, MVT::i16)};
  return DAG.getNode(AAPISD::SELECT_CC, Loc, Op.getValueType(), Ops);
}

EVT AAPTargetLowering::getSetCCResultType(const DataLayout &DL,
                                          LLVMContext &Context, EVT VT) const {
  if (!VT.isVector())
    return getPointerTy(DL);

  return VT.changeVectorElementTypeToInteger();
}

SDValue AAPTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc Loc(Op);

  SDValue Chain = Op.getOperand(0);

  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  // get equivalent AAP condition code
  AAPCC::CondCode TargetCC = getAAPCondCode(CC);

  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue BranchTarget = Op.getOperand(4);

  SDValue Ops[] = {Chain, DAG.getConstant(TargetCC, Loc, MVT::i16), LHS, RHS,
                   BranchTarget};
  return DAG.getNode(AAPISD::BR_CC, Loc, Op.getValueType(), Ops);
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

/// LowerCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
SDValue AAPTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  switch (CLI.CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    break;
  }

  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  // AAP target does not yet support tail call optimization.
  CLI.IsTailCall = false;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  EVT PtrTy = getPointerTy(DAG.getDataLayout());
  MachineFunction &MF = DAG.getMachineFunction();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_AAP);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    }

    // Arguments that can be passed on register must be kept in RegsToPass
    // vector
    if (VA.isRegLoc())
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    else {
      assert(VA.isMemLoc() && "Argument not register or memory");

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(
            Chain, DL, AAPRegisterInfo::getStackPtrRegister(), PtrTy);

      SDValue PtrOff =
          DAG.getNode(ISD::ADD, DL, PtrTy, StackPtr,
                      DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) {
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), DL, MVT::i16);
        MemOp = DAG.getMemcpy(
            Chain, DL, PtrOff, Arg, SizeNode, Flags.getByValAlign(),
            /*isVolatile*/ false,
            /*AlwaysInline*/ true,
            /*isTailCall*/ false, MachinePointerInfo(), MachinePointerInfo());
      } else
        MemOp = DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo());

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The Glue is
  // necessary since all emitted instructions must be stuck together.
  SDValue Glue;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  // If the callee is a global address or external symbol, convert it to a
  // target node so that legalize doesn't hack it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, PtrTy, 0);
  if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrTy);

  // The first call operand is the chain and the second is the target address.
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add the link register as the first operand
  Ops.push_back(DAG.getRegister(AAPRegisterInfo::getLinkRegister(), MVT::i16));

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  // Add the caller saved registers as a register mask operand to the call
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  assert(Mask && "No call preserved mask for the calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Glue the call to the argument copies, if any.
  if (Glue.getNode())
    Ops.push_back(Glue);

  // Emit the call.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(AAPISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  // Create the CALLSEQ_END node to mark the end of the call.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getConstant(NumBytes, DL, PtrTy, true),
                             DAG.getConstant(0, DL, PtrTy, true), Glue, DL);
  Glue = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCResInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());

  CCResInfo.AnalyzeCallResult(Ins, RetCC_AAP);

  // Copy all of the result registers out of their specified physreg.
  for (auto &VA : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getValVT(), Glue)
                .getValue(1);
    Glue = Chain.getValue(2);

    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//                      AAP Custom Instruction Emission
//===----------------------------------------------------------------------===//

MachineBasicBlock *
AAPTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *MBB) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Unexpected instruction for custom emission");
  case AAP::SELECT_CC:
    return EmitSELECT_CC(MI, MBB);
  case AAP::BR_CC:
    return EmitBR_CC(MI, MBB);
  }
}

MachineBasicBlock *
AAPTargetLowering::EmitSELECT_CC(MachineInstr &MI,
                                 MachineBasicBlock *MBB) const {
  DebugLoc DL = MI.getDebugLoc();
  const TargetInstrInfo &TII = *MBB->getParent()->getSubtarget().getInstrInfo();

  // Insert a diamond control flow pattern to handle the select, IE:
  //
  //     EntryMBB
  //      |  \
  //      |  FalseValueMBB
  //      | /
  //     SinkMBB

  const BasicBlock *BB = MBB->getBasicBlock();
  MachineFunction::iterator It = MBB->getIterator();
  ++It;

  MachineBasicBlock *EntryMBB = MBB;
  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *FalseValueMBB = MF->CreateMachineBasicBlock(BB);
  MachineBasicBlock *SinkMBB = MF->CreateMachineBasicBlock(BB);
  MF->insert(It, FalseValueMBB);
  MF->insert(It, SinkMBB);

  // Transfer remainder of entryMBB to sinkMBB
  SinkMBB->splice(SinkMBB->begin(), EntryMBB,
                  std::next(MachineBasicBlock::iterator(MI)), EntryMBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(EntryMBB);

  // Add false value and fallthrough blocks as successors
  EntryMBB->addSuccessor(FalseValueMBB);
  EntryMBB->addSuccessor(SinkMBB);

  AAPCC::CondCode CC = (AAPCC::CondCode)MI.getOperand(5).getImm();
  unsigned BranchOp = AAPInstrInfo::getBranchOpcodeFromCond(CC);

  unsigned InReg = MI.getOperand(0).getReg();
  unsigned LHSReg = MI.getOperand(1).getReg();
  unsigned RHSReg = MI.getOperand(2).getReg();
  BuildMI(EntryMBB, DL, TII.get(BranchOp))
      .addMBB(SinkMBB)
      .addReg(LHSReg)
      .addReg(RHSReg);

  FalseValueMBB->addSuccessor(SinkMBB);

  unsigned TrueValueReg = MI.getOperand(3).getReg();
  unsigned FalseValueReg = MI.getOperand(4).getReg();
  BuildMI(*SinkMBB, SinkMBB->begin(), DL, TII.get(AAP::PHI), InReg)
      .addReg(TrueValueReg)
      .addMBB(EntryMBB)
      .addReg(FalseValueReg)
      .addMBB(FalseValueMBB);

  MI.eraseFromParent();
  return SinkMBB;
}

MachineBasicBlock *AAPTargetLowering::EmitBR_CC(MachineInstr &MI,
                                                MachineBasicBlock *MBB) const {
  DebugLoc DL = MI.getDebugLoc();
  const TargetInstrInfo &TII = *MBB->getParent()->getSubtarget().getInstrInfo();

  AAPCC::CondCode CC = (AAPCC::CondCode)MI.getOperand(0).getImm();
  unsigned BranchOp = AAPInstrInfo::getBranchOpcodeFromCond(CC);

  unsigned LHSReg = MI.getOperand(1).getReg();
  unsigned RHSReg = MI.getOperand(2).getReg();
  MachineBasicBlock *TargetMBB = MI.getOperand(3).getMBB();

  BuildMI(*MBB, &MI, DL, TII.get(BranchOp))
      .addMBB(TargetMBB)
      .addReg(LHSReg)
      .addReg(RHSReg);

  MI.eraseFromParent();
  return MBB;
}
