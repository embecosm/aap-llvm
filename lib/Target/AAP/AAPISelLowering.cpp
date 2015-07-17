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
#include "AAP.h"
#include "AAPMachineFunctionInfo.h"
#include "AAPSubtarget.h"
#include "AAPTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "aap-lower"

AAPTargetLowering::AAPTargetLowering(const TargetMachine &TM,
                                     const AAPSubtarget &STI)
    : TargetLowering(TM) {

  // Set up the register classes.
  addRegisterClass(MVT::i16, &AAP::GR8RegClass);
  addRegisterClass(MVT::i16, &AAP::GR64RegClass);
  computeRegisterProperties(STI.getRegisterInfo());

  // Division is expensive
  setIntDivIsCheap(false);

  setStackPointerRegisterToSaveRestore(AAP::R1);
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  // We have post-incremented loads / stores.
  // setIndexedLoadAction(ISD::POST_INC, MVT::i8, Legal);
  // setIndexedLoadAction(ISD::POST_INC, MVT::i16, Legal);

  // Only basic load with zero extension i8 -> i16 is supported
  // Note: EXTLOAD promotion will trigger an assertion
  setLoadExtAction(ISD::EXTLOAD,  MVT::i8,  MVT::i1,  Promote);
  setLoadExtAction(ISD::EXTLOAD,  MVT::i16, MVT::i1,  Promote);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8,  MVT::i1,  Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, MVT::i1,  Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i8,  MVT::i1,  Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i1,  Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i8,  Expand);


  setOperationAction(ISD::GlobalAddress, MVT::i16,    Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  // Handle conditionals via brcc and selectcc
  setOperationAction(ISD::BRCOND, MVT::i8,    Expand);
  setOperationAction(ISD::BRCOND, MVT::i16,   Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);

  setOperationAction(ISD::SELECT, MVT::i8,    Expand);
  setOperationAction(ISD::SELECT, MVT::i16,   Expand);
  setOperationAction(ISD::SELECT, MVT::Other, Expand);

  setOperationAction(ISD::SETCC, MVT::i8,     Expand);
  setOperationAction(ISD::SETCC, MVT::i16,    Expand);
  setOperationAction(ISD::SETCC, MVT::Other,  Expand);

  setOperationAction(ISD::SELECT_CC, MVT::i8,   Promote);
  setOperationAction(ISD::SELECT_CC, MVT::i16,  Custom);
  setOperationAction(ISD::BR_CC,     MVT::i8,   Promote);
  setOperationAction(ISD::BR_CC,     MVT::i16,  Custom);

  // Currently no support for indirect branches
  setOperationAction(ISD::BRIND,     MVT::Other,  Expand);

  // No support for jump tables
  setOperationAction(ISD::JumpTable, MVT::i8,     Expand);
  setOperationAction(ISD::JumpTable, MVT::i16,    Expand);
  setOperationAction(ISD::BR_JT,     MVT::Other,  Expand);

  // vaarg
  setOperationAction(ISD::VASTART,  MVT::Other, Custom);
  setOperationAction(ISD::VAARG,    MVT::Other, Expand);
  setOperationAction(ISD::VAEND,    MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,   MVT::Other, Expand);

  // ALU operations unsupported by the architecture
  setOperationAction(ISD::SDIV,     MVT::i8,  Expand);
  setOperationAction(ISD::SDIV,     MVT::i16, Expand);
  setOperationAction(ISD::UDIV,     MVT::i8,  Expand);
  setOperationAction(ISD::UDIV,     MVT::i16, Expand);
  setOperationAction(ISD::UREM,     MVT::i8,  Expand);
  setOperationAction(ISD::UREM,     MVT::i16, Expand);
  setOperationAction(ISD::SREM,     MVT::i8,  Expand);
  setOperationAction(ISD::SREM,     MVT::i16, Expand);
  setOperationAction(ISD::SDIVREM,  MVT::i8,  Expand);
  setOperationAction(ISD::SDIVREM,  MVT::i16, Expand);
  setOperationAction(ISD::UDIVREM,  MVT::i8,  Expand);
  setOperationAction(ISD::UDIVREM,  MVT::i16, Expand);

  setOperationAction(ISD::MUL, MVT::i8,  Expand);
  setOperationAction(ISD::MUL, MVT::i16, Expand);
  setOperationAction(ISD::MULHS, MVT::i8,  Expand);
  setOperationAction(ISD::MULHS, MVT::i16, Expand);
  setOperationAction(ISD::MULHU, MVT::i8,  Expand);
  setOperationAction(ISD::MULHU, MVT::i16, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i8,  Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i16, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i8,  Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i16, Expand);
  
  // Use ADDE/SUBE
  setOperationAction(ISD::ADDC, MVT::i8,  Expand);
  setOperationAction(ISD::ADDC, MVT::i16, Expand);
  setOperationAction(ISD::SUBC, MVT::i8,  Expand);
  setOperationAction(ISD::SUBC, MVT::i16, Expand);

  setOperationAction(ISD::ROTL, MVT::i8,  Expand);
  setOperationAction(ISD::ROTL, MVT::i16, Expand);
  setOperationAction(ISD::ROTR, MVT::i8,  Expand);
  setOperationAction(ISD::ROTR, MVT::i16, Expand);

  setOperationAction(ISD::SHL_PARTS, MVT::i8,  Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i8,  Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i8,  Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i16, Expand);

  setOperationAction(ISD::BSWAP, MVT::i8,  Expand);
  setOperationAction(ISD::BSWAP, MVT::i16, Expand);

  setOperationAction(ISD::CTTZ,  MVT::i8,  Expand);
  setOperationAction(ISD::CTTZ,  MVT::i16, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF,  MVT::i8,  Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF,  MVT::i16, Expand);
  setOperationAction(ISD::CTLZ,  MVT::i8,  Expand);
  setOperationAction(ISD::CTLZ,  MVT::i16, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF,  MVT::i8,  Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF,  MVT::i16, Expand);
  setOperationAction(ISD::CTPOP, MVT::i8,  Expand);
  setOperationAction(ISD::CTPOP, MVT::i16, Expand);


  setMinFunctionAlignment(1);
  setPrefFunctionAlignment(2);
}

SDValue AAPTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:  return LowerGlobalAddress(Op, DAG);
  case ISD::BR_CC:          return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:      return LowerSELECT_CC(Op, DAG);
  case ISD::VASTART:        return LowerVASTART(Op, DAG);
  }
  llvm_unreachable("unimplemented operand");
}

//===----------------------------------------------------------------------===//
//                       AAP Inline Assembly Support
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "AAPGenCallingConv.inc"

/// For each argument in a function store the number of pieces it is composed
/// of.
template <typename ArgT>
static void ParseFunctionArgs(const SmallVectorImpl<ArgT> &Args,
                              SmallVectorImpl<unsigned> &Out) {
  unsigned CurrentArgIndex = ~0U;
  for (unsigned i = 0, e = Args.size(); i != e; i++) {
    if (CurrentArgIndex == Args[i].OrigArgIndex) {
      Out.back()++;
    } else {
      Out.push_back(1);
      CurrentArgIndex++;
    }
  }
}

SDValue AAPTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return LowerCCCArguments(Chain, CallConv, isVarArg, Ins, dl, DAG, InVals);
  }
}

SDValue AAPTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &dl = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  // AAP target does not yet support tail call optimization.
  isTailCall = false;

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    return LowerCCCCallTo(Chain, Callee, CallConv, isVarArg, isTailCall, Outs,
                          OutVals, Ins, dl, DAG, InVals);
  }
}

/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
// FIXME: struct returns
SDValue AAPTargetLowering::LowerCCCArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  AAPMachineFunctionInfo *FuncInfo = MF.getInfo<AAPMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_AAP);

  // Create frame index for the start of the first vararg value
  if (isVarArg) {
    unsigned Offset = CCInfo.getNextStackOffset();
    FuncInfo->setVarArgsFrameIndex(MFI->CreateFixedObject(1, Offset, true));
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
               << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
        llvm_unreachable(0);
      }
      case MVT::i16:
        unsigned VReg = RegInfo.createVirtualRegister(&AAP::GR64RegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

        // If this is an 8-bit value, it is really passed promoted to 16
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);
      }
    } else {
      // Sanity check
      assert(VA.isMemLoc());

      SDValue InVal;
      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) {
        int FI = MFI->CreateFixedObject(Flags.getByValSize(),
                                        VA.getLocMemOffset(), true);
        InVal = DAG.getFrameIndex(FI, getPointerTy());
      } else {
        // Load the argument to a virtual register
        unsigned ObjSize = VA.getLocVT().getSizeInBits() / 8;
        if (ObjSize > 2) {
          errs() << "LowerFormalArguments Unhandled argument type: "
                 << EVT(VA.getLocVT()).getEVTString() << "\n";
        }
        // Create the frame index object for this incoming parameter...
        int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

        // Create the SelectionDAG nodes corresponding to a load
        // from this parameter
        SDValue FIN = DAG.getFrameIndex(FI, MVT::i16);
        InVal = DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
                            MachinePointerInfo::getFixedStack(FI), false, false,
                            false, 0);
      }

      InVals.push_back(InVal);
    }
  }

  return Chain;
}

SDValue
AAPTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               SDLoc dl, SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_AAP);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Add the link register as the first operand
  RetOps.push_back(
    DAG.getRegister(AAPRegisterInfo::getLinkRegister(), MVT::i16));

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(AAPISD::RET_FLAG, dl, {MVT::Other, MVT::i16}, RetOps);
}

/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
// TODO: sret.
SDValue AAPTargetLowering::LowerCCCCallTo(
    SDValue Chain, SDValue Callee, CallingConv::ID CallConv, bool isVarArg,
    bool isTailCall, const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_AAP);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(
      Chain, DAG.getConstant(NumBytes, getPointerTy(), true), dl);

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
      Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl,
                                      AAPRegisterInfo::getStackPtrRegister(),
                                      getPointerTy());

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(), StackPtr,
                                   DAG.getIntPtrConstant(VA.getLocMemOffset()));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) {
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i16);
        MemOp = DAG.getMemcpy(
            Chain, dl, PtrOff, Arg, SizeNode, Flags.getByValAlign(),
            /*isVolatile*/ false,
            /*AlwaysInline=*/true, MachinePointerInfo(), MachinePointerInfo());
      } else {
        MemOp = DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo(),
                             false, false, 0);
      }

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains[0]);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  // Add the link register as a return
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  Ops.push_back(DAG.getRegister(AAPRegisterInfo::getLinkRegister(), MVT::i16));


  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(AAPISD::CALL, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain =
      DAG.getCALLSEQ_END(Chain, DAG.getConstant(NumBytes, getPointerTy(), true),
                         DAG.getConstant(0, getPointerTy(), true), InFlag, dl);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl, DAG,
                         InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue AAPTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_AAP);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag)
                .getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

// Get the AAP specific condition code for a given CondCode DAG node.
// The first element of the pair is a bool dictating whether the returned
// condition code requires the operands to be swapped.
// The second element of the pair is the equivalent AAP condition code
static std::pair<bool, AAPCC::CondCode> getAAPCondCode(ISD::CondCode CC) {
  AAPCC::CondCode TargetCC;
  bool shouldSwapOps;

  switch (CC) {
  // These have a direct equivalent
  case ISD::SETEQ:
    TargetCC = AAPCC::COND_EQ;
    break;
  case ISD::SETNE:
    TargetCC = AAPCC::COND_NE;
    break;
  case ISD::SETLT:
    TargetCC = AAPCC::COND_LTS;
    break;
  case ISD::SETGT:
    TargetCC = AAPCC::COND_GTS;
    break;
  case ISD::SETULT:
    TargetCC = AAPCC::COND_LTU;
    break;
  case ISD::SETUGT:
    TargetCC = AAPCC::COND_GTU;
    break;

  // These require lhs/rhs to be swapped, therefore the condition returned
  // is inverted
  case ISD::SETLE:
    TargetCC = AAPCC::COND_GTS;
    break;
  case ISD::SETGE:
    TargetCC = AAPCC::COND_LTS;
    break;
  case ISD::SETULE:
    TargetCC = AAPCC::COND_GTU;
    break;
  case ISD::SETUGE:
    TargetCC = AAPCC::COND_LTU;
    break;
  default:
    llvm_unreachable("Unknown condition for brcc lowering");
  }

  shouldSwapOps = (CC == ISD::SETLE) || (CC == ISD::SETGE) ||
                  (CC == ISD::SETULE) || (CC == ISD::SETUGE);

  return std::make_pair(shouldSwapOps, TargetCC);
}

// Map the generic BR_CC instruction to a specific branch instruction based
// on the provided AAP condition code
static unsigned getBranchOpForCondition(int branchOp, AAPCC::CondCode CC) {
  assert(branchOp == AAP::BR_CC || branchOp == AAP::BR_CC);

  if (branchOp == AAP::BR_CC) {
    switch (CC) {
    case AAPCC::COND_EQ:
      branchOp = AAP::BEQ_;
      break;
    case AAPCC::COND_NE:
      branchOp = AAP::BNE_;
      break;
    case AAPCC::COND_LTS:
      branchOp = AAP::BLTS_;
      break;
    case AAPCC::COND_GTS:
      branchOp = AAP::BGTS_;
      break;
    case AAPCC::COND_LTU:
      branchOp = AAP::BLTU_;
      break;
    case AAPCC::COND_GTU:
      branchOp = AAP::BGTU_;
      break;
    default:
      llvm_unreachable("Unknown condition code!");
    }
  } else {
    switch (CC) {
    case AAPCC::COND_EQ:
      branchOp = AAP::BEQ_;
      break;
    case AAPCC::COND_NE:
      branchOp = AAP::BNE_;
      break;
    case AAPCC::COND_LTS:
      branchOp = AAP::BLTS_;
      break;
    case AAPCC::COND_GTS:
      branchOp = AAP::BGTS_;
      break;
    case AAPCC::COND_LTU:
      branchOp = AAP::BLTU_;
      break;
    case AAPCC::COND_GTU:
      branchOp = AAP::BGTU_;
      break;
    default:
      llvm_unreachable("Unknown condition code!");
    }
  }
  return branchOp;
}

SDValue AAPTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue BranchTarget = Op.getOperand(4);

  // get equivalent AAP condition code, swap operands if necessary
  std::pair<bool, AAPCC::CondCode> CCPair = getAAPCondCode(CC);
  bool SwapOperands = CCPair.first;
  AAPCC::CondCode TargetCC = CCPair.second;

  SmallVector<SDValue, 5> Ops;
  Ops.push_back(Chain);
  Ops.push_back(DAG.getConstant(TargetCC, MVT::i16));
  if (!SwapOperands) {
    Ops.push_back(LHS);
    Ops.push_back(RHS);
  } else {
    Ops.push_back(RHS);
    Ops.push_back(LHS);
  }
  Ops.push_back(BranchTarget);
  return DAG.getNode(AAPISD::BR_CC, dl, Op.getValueType(), Ops);
}

SDValue AAPTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);

  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueValue = Op.getOperand(2);
  SDValue FalseValue = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  // get equivalent AAP condition code, swap operands if necessary
  std::pair<bool, AAPCC::CondCode> CCPair = getAAPCondCode(CC);
  bool SwapOperands = CCPair.first;
  AAPCC::CondCode TargetCC = CCPair.second;

  SmallVector<SDValue, 5> Ops;
  if (!SwapOperands) {
    Ops.push_back(LHS);
    Ops.push_back(RHS);
  } else {
    Ops.push_back(RHS);
    Ops.push_back(LHS);
  }
  Ops.push_back(TrueValue);
  Ops.push_back(FalseValue);
  Ops.push_back(DAG.getConstant(TargetCC, MVT::i16));
  return DAG.getNode(AAPISD::SELECT_CC, dl, Op.getValueType(), Ops);
}

SDValue AAPTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  AAPMachineFunctionInfo *MFI = MF.getInfo<AAPMachineFunctionInfo>();

  // Frame index of first vaarg argument
  SDValue FrameIndex = DAG.getFrameIndex(MFI->getVarArgsFrameIndex(),
                                         getPointerTy());
  const Value *Src = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Create a store of the frame index to the location operand
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex,
                      Op.getOperand(1), MachinePointerInfo(Src),
                      false, false, 0);
}

SDValue AAPTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result =
      DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(), Offset);
  return DAG.getNode(AAPISD::Wrapper, SDLoc(Op), getPointerTy(), Result);
}


//===----------------------------------------------------------------------===//
//                      AAP Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target
TargetLowering::ConstraintType
AAPTargetLowering::getConstraintType(const std::string &Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'r':
      return C_RegisterClass;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass*>
AAPTargetLowering::
getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                             const std::string &Constraint,
                             MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'r':
      // General purpose registers
      return std::make_pair(0U, &AAP::GR64RegClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}


MachineBasicBlock *
AAPTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                               MachineBasicBlock *MBB) const {
  switch (MI->getOpcode()) {
  case AAP::BR_CC:
  case AAP::BR_CC_short:
    return emitBrCC(MI, MBB);
  case AAP::SELECT_CC:
    return emitSelectCC(MI, MBB);
  default:
    llvm_unreachable("Unexpected instruction for custom insertion");
  }
}

MachineBasicBlock *AAPTargetLowering::emitBrCC(MachineInstr *MI,
                                               MachineBasicBlock *MBB) const {
  const TargetInstrInfo *TII = MBB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();
  AAPCC::CondCode CC = (AAPCC::CondCode)MI->getOperand(0).getImm();

  unsigned lhsReg = MI->getOperand(1).getReg();
  unsigned rhsReg = MI->getOperand(2).getReg();
  MachineBasicBlock *targetMBB = MI->getOperand(3).getMBB();

  unsigned branchOp = getBranchOpForCondition(MI->getOpcode(), CC);
  BuildMI(*MBB, MI, dl, TII->get(branchOp))
      .addMBB(targetMBB)
      .addReg(lhsReg)
      .addReg(rhsReg);

  MI->eraseFromParent();
  return MBB;
}

MachineBasicBlock *
AAPTargetLowering::emitSelectCC(MachineInstr *MI,
                                MachineBasicBlock *MBB) const {
  const TargetInstrInfo *TII = MBB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();

  // insert a diamond control flow pattern to handle the select
  const BasicBlock *llvmBB = MBB->getBasicBlock();
  MachineFunction::iterator It = MBB;
  ++It;

  MachineBasicBlock *entryMBB = MBB;
  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *trueValueMBB = MF->CreateMachineBasicBlock(llvmBB);
  MachineBasicBlock *sinkMBB = MF->CreateMachineBasicBlock(llvmBB);
  MF->insert(It, trueValueMBB);
  MF->insert(It, sinkMBB);

  // Transfer remainder of entryBB to sinkMBB
  sinkMBB->splice(sinkMBB->begin(), entryMBB,
                  std::next(MachineBasicBlock::iterator(MI)), entryMBB->end());
  sinkMBB->transferSuccessorsAndUpdatePHIs(entryMBB);

  // Add true value and fallthrough blocks as successors
  entryMBB->addSuccessor(trueValueMBB);
  entryMBB->addSuccessor(sinkMBB);

  // Choose the branch instruction to use based on the condition code
  // For now, always use the long instructions
  AAPCC::CondCode CC = (AAPCC::CondCode)MI->getOperand(5).getImm();
  unsigned branchOp;
  switch (CC) {
  case AAPCC::COND_EQ:
    branchOp = AAP::BEQ_;
    break;
  case AAPCC::COND_NE:
    branchOp = AAP::BNE_;
    break;
  case AAPCC::COND_LTS:
    branchOp = AAP::BLTS_;
    break;
  case AAPCC::COND_GTS:
    branchOp = AAP::BGTS_;
    break;
  case AAPCC::COND_LTU:
    branchOp = AAP::BLTU_;
    break;
  case AAPCC::COND_GTU:
    branchOp = AAP::BGTU_;
    break;
  default:
    llvm_unreachable("Unknown condition code!");
  }

  unsigned inReg = MI->getOperand(0).getReg();
  unsigned lhsReg = MI->getOperand(1).getReg();
  unsigned rhsReg = MI->getOperand(2).getReg();
  BuildMI(entryMBB, dl, TII->get(branchOp))
      .addMBB(trueValueMBB)
      .addReg(lhsReg)
      .addReg(rhsReg);

  trueValueMBB->addSuccessor(sinkMBB);

  unsigned trueValueReg = MI->getOperand(3).getReg();
  unsigned falseValueReg = MI->getOperand(4).getReg();
  BuildMI(*sinkMBB, sinkMBB->begin(), dl, TII->get(AAP::PHI), inReg)
      .addReg(trueValueReg)
      .addMBB(trueValueMBB)
      .addReg(falseValueReg)
      .addMBB(entryMBB);

  MI->eraseFromParent();
  return sinkMBB;
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
