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
    : TargetLowering(TM), Subtarget(STI) {

  // Set up the register classes.
  addRegisterClass(MVT::i16, &AAP::GR8RegClass);
  addRegisterClass(MVT::i16, &AAP::GR64RegClass);
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(AAPRegisterInfo::getStackPtrRegister());
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  // Only basic load with zero extension i8 -> i16 is supported
  // Note: EXTLOAD promotion will trigger an assertion
  setLoadExtAction(ISD::EXTLOAD,  MVT::i8,  MVT::i1, Promote);
  setLoadExtAction(ISD::EXTLOAD,  MVT::i16, MVT::i1, Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8,  MVT::i1, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, MVT::i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8,  MVT::i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i8, Expand);

  setOperationAction(ISD::GlobalAddress,  MVT::i16, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i16, Custom);
  setOperationAction(ISD::BlockAddress,   MVT::i16, Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  // Handle conditionals via brcc and selectcc
  setOperationAction(ISD::BRCOND,    MVT::i16,   Expand);
  setOperationAction(ISD::BRCOND,    MVT::Other, Expand);
  setOperationAction(ISD::SELECT,    MVT::i16,   Expand);
  setOperationAction(ISD::SETCC,     MVT::i16,   Expand);
  setOperationAction(ISD::SETCC,     MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i16,   Custom);
  setOperationAction(ISD::BR_CC,     MVT::i16,   Custom);

  // Expand some condition codes which are not natively supported
  setCondCodeAction(ISD::SETGT,      MVT::i16,   Expand);
  setCondCodeAction(ISD::SETGE,      MVT::i16,   Expand);
  setCondCodeAction(ISD::SETUGT,     MVT::i16,   Expand);
  setCondCodeAction(ISD::SETUGE,     MVT::i16,   Expand);

  // Currently no support for indirect branches
  setOperationAction(ISD::BRIND,     MVT::Other, Expand);

  // No support for jump tables
  setOperationAction(ISD::JumpTable, MVT::i16,   Expand);
  setOperationAction(ISD::BR_JT,     MVT::Other, Expand);

  // vaarg
  setOperationAction(ISD::VASTART,   MVT::Other, Custom);
  setOperationAction(ISD::VAARG,     MVT::Other, Expand);
  setOperationAction(ISD::VAEND,     MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,    MVT::Other, Expand);

  // ALU operations unsupported by the architecture
  setOperationAction(ISD::SDIV,      MVT::i16,   Expand);
  setOperationAction(ISD::UDIV,      MVT::i16,   Expand);
  setOperationAction(ISD::UREM,      MVT::i16,   Expand);
  setOperationAction(ISD::SREM,      MVT::i16,   Expand);
  setOperationAction(ISD::SDIVREM,   MVT::i16,   Expand);
  setOperationAction(ISD::UDIVREM,   MVT::i16,   Expand);

  setOperationAction(ISD::MUL,       MVT::i16,   Expand);
  setOperationAction(ISD::MULHS,     MVT::i16,   Expand);
  setOperationAction(ISD::MULHU,     MVT::i16,   Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i16,   Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i16,   Expand);

  // Use ADDE/SUBE
  setOperationAction(ISD::SUBC, MVT::i16, Expand);

  setOperationAction(ISD::ROTL,      MVT::i16, Expand);
  setOperationAction(ISD::ROTR,      MVT::i16, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i16, Expand);

  setOperationAction(ISD::BSWAP,           MVT::i16, Expand);
  setOperationAction(ISD::CTTZ,            MVT::i16, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i16, Expand);
  setOperationAction(ISD::CTLZ,            MVT::i16, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i16, Expand);
  setOperationAction(ISD::CTPOP,           MVT::i16, Expand);

  setOperationAction(ISD::FLT_ROUNDS_, MVT::i32, Custom);

  // Custom DAGCombine
  setTargetDAGCombine(ISD::ADD);

  setMinFunctionAlignment(1);
  setPrefFunctionAlignment(2);
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

EVT AAPTargetLowering::getSetCCResultType(const DataLayout &DL,
                                          LLVMContext &Context, EVT VT) const {
  if (!VT.isVector()) {
    return MVT::i16;
  }
  return VT.changeVectorElementTypeToInteger();
}

//===----------------------------------------------------------------------===//
//                      Custom DAG Combine Implementation
//===----------------------------------------------------------------------===//

SDValue AAPTargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  case ISD::ADD:
    return PerformADDCombine(N, DCI);
  default:
    break;
  }
  return SDValue();
}

SDValue AAPTargetLowering::PerformADDCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;

  // fold add -ve -> sub +ve
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  SDLoc DL(N);

  ConstantSDNode *Const = dyn_cast<ConstantSDNode>(RHS);
  if (!Const) {
    return SDValue();
  }
  int64_t Value = Const->getSExtValue();
  if (Value >= 0) {
    return SDValue();
  }

  RHS = DAG.getConstant(-Value, DL, RHS.getValueType());

  SDValue Res = DAG.getNode(ISD::SUB, DL, N->getValueType(0), LHS, RHS);
  DAG.ReplaceAllUsesWith(N, Res.getNode());
  return SDValue(N, 0);
}

//===----------------------------------------------------------------------===//
//                       Custom Lowering Implementation
//===----------------------------------------------------------------------===//

SDValue AAPTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::ExternalSymbol:
    return LowerExternalSymbol(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  }
  llvm_unreachable("unimplemented operand");
}

void AAPTargetLowering::ReplaceNodeResults(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  case ISD::FLT_ROUNDS_:
    // FLT_ROUNDS has an i32 result type, and can't be expanded. Just lower
    // to -1, which corresponds to an unknown default rounding direction.
    Results.push_back(DAG.getTargetConstant(-1, SDLoc(N), MVT::i32));
    break;
  default:
    llvm_unreachable("Unhandled node in ReplaceNodeResults");
  }
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
    llvm_unreachable("Unknown condition for brcc lowering");
    return AAPCC::COND_INVALID;
  }
}

SDValue AAPTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue BranchTarget = Op.getOperand(4);

  // get equivalent AAP condition code
  AAPCC::CondCode TargetCC = getAAPCondCode(CC);

  SDValue Ops[] = {Chain, DAG.getConstant(TargetCC, DL, MVT::i16), LHS, RHS,
                   BranchTarget};
  return DAG.getNode(AAPISD::BR_CC, DL, Op.getValueType(), Ops);
}

SDValue AAPTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);

  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueValue = Op.getOperand(2);
  SDValue FalseValue = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  // get equivalent AAP condition code
  AAPCC::CondCode TargetCC = getAAPCondCode(CC);

  SDValue Ops[] = {LHS, RHS, TrueValue, FalseValue,
                   DAG.getConstant(TargetCC, DL, MVT::i16)};
  return DAG.getNode(AAPISD::SELECT_CC, DL, Op.getValueType(), Ops);
}

SDValue AAPTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  AAPMachineFunctionInfo *MFI = MF.getInfo<AAPMachineFunctionInfo>();
  const DataLayout &DL = DAG.getDataLayout();

  // Frame index of first vaarg argument
  SDValue FrameIndex =
      DAG.getFrameIndex(MFI->getVarArgsFrameIndex(), getPointerTy(DL));
  const Value *Src = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Create a store of the frame index to the location operand
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex, Op.getOperand(1),
                      MachinePointerInfo(Src));
}

SDValue AAPTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  const DataLayout DL = DAG.getDataLayout();
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();
  SDValue Result =
      DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(DL), Offset);
  return DAG.getNode(AAPISD::Wrapper, SDLoc(Op), getPointerTy(DL), Result);
}

SDValue AAPTargetLowering::LowerExternalSymbol(SDValue Op,
                                               SelectionDAG &DAG) const {
  const DataLayout DL = DAG.getDataLayout();
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result = DAG.getTargetExternalSymbol(Sym, getPointerTy(DL));
  return DAG.getNode(AAPISD::Wrapper, SDLoc(Op), getPointerTy(DL), Result);
}

SDValue AAPTargetLowering::LowerBlockAddress(SDValue Op,
                                             SelectionDAG &DAG) const {
  const DataLayout DL = DAG.getDataLayout();
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result = DAG.getTargetBlockAddress(BA, getPointerTy(DL));
  return DAG.getNode(AAPISD::Wrapper, SDLoc(Op), getPointerTy(DL), Result);
}

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
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return LowerCCCArguments(Chain, CallConv, isVarArg, Ins, DL, DAG, InVals);
  }
}

SDValue AAPTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
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
                          OutVals, Ins, DL, DAG, InVals);
  }
}

/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
SDValue AAPTargetLowering::LowerCCCArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
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
    FuncInfo->setVarArgsFrameIndex(MFI.CreateFixedObject(1, Offset, true));
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
        SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        // If this is an 8-bit value, it is really passed promoted to 16
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), ArgValue);

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
        InVal = DAG.getLoad(VA.getLocVT(), DL, Chain, FIN,
                            MachinePointerInfo::getFixedStack(MF, FI));
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
                               const SDLoc &DL, SelectionDAG &DAG) const {

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

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(AAPISD::RET_FLAG, DL, {MVT::Other, MVT::i16}, RetOps);
}

/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
SDValue AAPTargetLowering::LowerCCCCallTo(
    SDValue Chain, SDValue Callee, CallingConv::ID CallConv, bool isVarArg,
    bool isTailCall, const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  const DataLayout &TD = DAG.getDataLayout();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_AAP);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(
      Chain, DAG.getConstant(NumBytes, DL, getPointerTy(TD), true), DL);

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

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, DL,
                                      AAPRegisterInfo::getStackPtrRegister(),
                                      getPointerTy(TD));

      SDValue PtrOff =
          DAG.getNode(ISD::ADD, DL, getPointerTy(TD), StackPtr,
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
      } else {
        MemOp = DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo());
      }

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, DL, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
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
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  assert(Mask && "No call preserved mask for the calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(AAPISD::CALL, DL, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(
      Chain, DAG.getConstant(NumBytes, DL, getPointerTy(TD), true),
      DAG.getConstant(0, DL, getPointerTy(TD), true), InFlag, DL);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, DL, DAG,
                         InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue AAPTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_AAP);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, DL, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag)
                .getValue(1);
    InFlag = Chain.getValue(2);
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
  case AAP::BR_CC:
    return emitBrCC(MI, MBB);
  case AAP::SELECT_CC:
    return emitSelectCC(MI, MBB);
  default:
    llvm_unreachable("Unexpected instruction for custom insertion");
  }
}

MachineBasicBlock *AAPTargetLowering::emitBrCC(MachineInstr &MI,
                                               MachineBasicBlock *MBB) const {
  const auto &STI = MBB->getParent()->getSubtarget();
  const auto &TII = *static_cast<const AAPInstrInfo*>(STI.getInstrInfo());
  DebugLoc DL = MI.getDebugLoc();
  AAPCC::CondCode CC = (AAPCC::CondCode)MI.getOperand(0).getImm();

  unsigned LhsReg = MI.getOperand(1).getReg();
  unsigned RhsReg = MI.getOperand(2).getReg();
  MachineBasicBlock *TargetMBB = MI.getOperand(3).getMBB();

  unsigned BranchOp = TII.getBranchOpcodeFromCond(CC);
  BuildMI(*MBB, &MI, DL, TII.get(BranchOp))
      .addMBB(TargetMBB)
      .addReg(LhsReg)
      .addReg(RhsReg);

  MI.eraseFromParent();
  return MBB;
}

MachineBasicBlock *
AAPTargetLowering::emitSelectCC(MachineInstr &MI,
                                MachineBasicBlock *MBB) const {
  const auto &STI = MBB->getParent()->getSubtarget();
  const auto &TII = *static_cast<const AAPInstrInfo*>(STI.getInstrInfo());
  DebugLoc DL = MI.getDebugLoc();

  // insert a diamond control flow pattern to handle the select
  const BasicBlock *BB = MBB->getBasicBlock();
  MachineFunction::iterator It = MBB->getIterator();
  ++It;

  MachineBasicBlock *EntryMBB = MBB;
  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *FalseValueMBB = MF->CreateMachineBasicBlock(BB);
  MachineBasicBlock *SinkMBB = MF->CreateMachineBasicBlock(BB);
  MF->insert(It, FalseValueMBB);
  MF->insert(It, SinkMBB);

  // Transfer remainder of entryBB to sinkMBB
  SinkMBB->splice(SinkMBB->begin(), EntryMBB,
                  std::next(MachineBasicBlock::iterator(MI)), EntryMBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(EntryMBB);

  // Add false value and fallthrough blocks as successors
  EntryMBB->addSuccessor(FalseValueMBB);
  EntryMBB->addSuccessor(SinkMBB);

  AAPCC::CondCode CC = (AAPCC::CondCode)MI.getOperand(5).getImm();
  unsigned BranchOp = TII.getBranchOpcodeFromCond(CC);

  unsigned InReg = MI.getOperand(0).getReg();
  unsigned LhsReg = MI.getOperand(1).getReg();
  unsigned RhsReg = MI.getOperand(2).getReg();
  BuildMI(EntryMBB, DL, TII.get(BranchOp))
      .addMBB(SinkMBB)
      .addReg(LhsReg)
      .addReg(RhsReg);

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

//===----------------------------------------------------------------------===//
//                      AAP Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target
TargetLowering::ConstraintType
AAPTargetLowering::getConstraintType(StringRef Constraint) const {
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

std::pair<unsigned, const TargetRegisterClass *>
AAPTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef Constraint,
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
