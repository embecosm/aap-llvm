//===-- AAPAsmParser.cpp - Parse AAP assembly to MCInst instructions ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AAP.h"
#include "MCTargetDesc/AAPMCTargetDesc.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace {
struct AAPOperand;

class AAPAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
  MCAsmParser &getParser() const { return Parser; }
  MCAsmLexer &getLexer() const { return Parser.getLexer(); }
  MCSubtargetInfo &STI;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool matchingInlineAsm) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc);

  std::unique_ptr<AAPOperand> ParseRegister(unsigned &RegNo);
  std::unique_ptr<AAPOperand> ParseImmediate();
  std::unique_ptr<AAPOperand> ParseMemSrc();

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID);

  bool ParseOperand(OperandVector &Operands);

// Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "AAPGenAsmMatcher.inc"

public:
  AAPAsmParser(MCSubtargetInfo &sti, MCAsmParser &_Parser,
               const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(), Parser(_Parser), STI(sti) {
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

/// AAPOperand - Instances of this class represented a parsed machine
/// instruction
struct AAPOperand : public MCParsedAsmOperand {

  enum KindTy { Token, Register, Immediate, MemSrc } Kind;

  SMLoc StartLoc, EndLoc;
  union {
    struct {
      const char *Data;
      unsigned Length;
    } Tok;
    struct {
      unsigned RegNum;
    } Reg;
    struct {
      const MCExpr *Val;
    } Imm;
    struct {
      unsigned RegNum;
      const MCExpr *Offset;
    } Mem;
  };

  AAPOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

public:
  AAPOperand(const AAPOperand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;
    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;
    switch (Kind) {
    case Register:
      Reg = o.Reg;
      break;
    case Immediate:
      Imm = o.Imm;
      break;
    case Token:
      Tok = o.Tok;
      break;
    case MemSrc:
      Mem = o.Mem;
    }
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const { return StartLoc; }

  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const { return EndLoc; }

  unsigned getReg() const {
    assert(Kind == Register && "Invalid type access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert(Kind == Immediate && "Invalid type access!");
    return Imm.Val;
  }

  StringRef getToken() const {
    assert(Kind == Token && "Invalid type access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  unsigned getMemSrcReg() const {
    assert(Kind == MemSrc && "Invalid type access!");
    return Mem.RegNum;
  }

  const MCExpr *getMemSrcImm() const {
    assert(Kind == MemSrc && "Invalid type access!");
    return Mem.Offset;
  }

  // Functions for testing operand type
  bool isReg() const { return Kind == Register; }
  bool isImm() const { return Kind == Immediate; }
  bool isToken() const { return Kind == Token; }
  bool isMemSrc() const { return Kind == MemSrc; }
  bool isMem() const { return false; }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediates where possible. Null MCExpr = 0
    if (Expr == 0)
      Inst.addOperand(MCOperand::CreateImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::CreateImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::CreateExpr(Expr));
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::CreateReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addMemSrcOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::CreateReg(getMemSrcReg()));
    addExpr(Inst, getMemSrcImm());
  }

  // FIXME: Implement this
  void print(raw_ostream &OS) const {}

  static std::unique_ptr<AAPOperand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = make_unique<AAPOperand>(Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<AAPOperand> CreateReg(unsigned RegNo, SMLoc S,
                                               SMLoc E) {
    auto Op = make_unique<AAPOperand>(Register);
    Op->Reg.RegNum = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<AAPOperand> CreateImm(const MCExpr *Val, SMLoc S,
                                               SMLoc E) {
    auto Op = make_unique<AAPOperand>(Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<AAPOperand>
  CreateMemSrc(unsigned RegNo, const MCExpr *Offset, SMLoc S, SMLoc E) {
    auto Op = make_unique<AAPOperand>(MemSrc);
    Op->Mem.RegNum = RegNo;
    Op->Mem.Offset = Offset;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }
};
} // end anonymous namespace.

// Auto-generated by TableGen
static unsigned MatchRegisterName(StringRef Name);
static const char *getSubtargetFeatureName(uint64_t Val);

bool AAPAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                           OperandVector &Operands,
                                           MCStreamer &Out, uint64_t &ErrorInfo,
                                           bool matchingInlineAsm) {
  MCInst Inst;
  SMLoc ErrorLoc;

  switch (MatchInstructionImpl(Operands, Inst, ErrorInfo, matchingInlineAsm)) {
  default:
    break;
  case Match_Success:
    Out.EmitInstruction(Inst, STI);
    return false;
  case Match_MissingFeature: {
    assert(ErrorInfo && "Unknown missing feature!");
    std::string Msg = "Use of this instruction requires:";
    unsigned Mask = 1;
    for (unsigned i = 0; i < (sizeof(ErrorInfo) * 8 - 1); ++i) {
      if (ErrorInfo & Mask) {
        Msg += " ";
        Msg += getSubtargetFeatureName(ErrorInfo & Mask);
      }
      Mask <<= 1;
    }
    return Error(IDLoc, Msg);
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "Unrecognized instruction mnemonic");
  case Match_InvalidOperand:
    ErrorLoc = IDLoc;
    if (ErrorInfo != ~0U) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "Too few operands for instruction");

      ErrorLoc = ((AAPOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }
    return Error(IDLoc, "Invalid operand for instruction");
  }

  llvm_unreachable("Unknown match type detected!");
}

bool AAPAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                 SMLoc &EndLoc) {
  return (ParseRegister(RegNo) == nullptr);
}

std::unique_ptr<AAPOperand> AAPAsmParser::ParseRegister(unsigned &RegNo) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  switch (getLexer().getKind()) {
  default:
    return nullptr;
  case AsmToken::Identifier:
    std::string lowerCaseStr = getLexer().getTok().getString().lower();

    RegNo = MatchRegisterName(lowerCaseStr);
    if (RegNo == 0) {
      return nullptr;
    }
    getLexer().Lex();
    return AAPOperand::CreateReg(RegNo, S, E);
  }
  return nullptr;
}

std::unique_ptr<AAPOperand> AAPAsmParser::ParseImmediate() {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  const MCExpr *EVal;
  if (!getParser().parseExpression(EVal, E)) {
    return AAPOperand::CreateImm(EVal, S, E);
  }
  return nullptr;

  /*
    const MCExpr *EVal;
    switch(getLexer().getKind()) {
      default: return 0;
      case AsmToken::Plus:
      case AsmToken::Minus:
      case AsmToken::Integer:
        if (getParser().parseExpression(EVal))
          return 0;

        int64_t ans;
        EVal->EvaluateAsAbsolute(ans);
        return AAPOperand::CreateImm(EVal, S, E);
    }
  */
}

std::unique_ptr<AAPOperand> AAPAsmParser::ParseMemSrc() {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  unsigned RegNo;
  std::unique_ptr<AAPOperand> RegOp = ParseRegister(RegNo);
  if (!RegOp) {
    Error(Parser.getTok().getLoc(), "Missing register in memsrc operand");
    return nullptr;
  }
  RegNo = RegOp->getReg();

  if (!getLexer().is(AsmToken::Comma)) {
    Error(Parser.getTok().getLoc(), "Missing ',' separator in memsrc operand");
    return nullptr;
  }
  Parser.Lex();

  std::unique_ptr<AAPOperand> ImmOp = ParseImmediate();
  if (!ImmOp) {
    Error(Parser.getTok().getLoc(), "Missing immediate in memsrc operand");
    return nullptr;
  }
  const MCExpr *ImmVal = ImmOp->getImm();

  return AAPOperand::CreateMemSrc(RegNo, ImmVal, S, E);
}

/// Looks at a token type and creates the relevant operand
/// from this information, adding to Operands.
/// If operand was parsed, returns false, else true.
bool AAPAsmParser::ParseOperand(OperandVector &Operands) {
  std::unique_ptr<AAPOperand> Op = nullptr;

  // Attempt to parse token as register
  unsigned RegNo;
  Op = ParseRegister(RegNo);
  if (Op) {
    Operands.push_back(std::move(Op));
    return false;
  }

  // Attempt to parse token as a memsrc operand
  // If we see a left bracket, that implies a memsrc follows
  if (getLexer().getKind() == AsmToken::LParen) {
    SMLoc S = Parser.getTok().getLoc();
    Operands.push_back(AAPOperand::CreateToken("(", S));
    Parser.Lex(); // Eat '('

    Op = ParseMemSrc();

    if (Op) {
      Operands.push_back(std::move(Op));

      S = Parser.getTok().getLoc();
      if (getLexer().getKind() == AsmToken::RParen) {
        Operands.push_back(AAPOperand::CreateToken(")", S));
        Parser.Lex(); // Eat ')'
      } else {
        Error(S, "Missing closing brace for memsrc operand");
        return true;
      }
      return false;
    }
  }

  // Attempt to parse token as an immediate
  Op = ParseImmediate();
  if (Op) {
    Operands.push_back(std::move(Op));
    return false;
  }

  // An immediate operand will be an expression
  // if (!Op) {
  //  const MCExpr *EVal;
  //  SMLoc S, E;
  //  if (!getParser().parseExpression(EVal, E)) {
  //    Op = AAPOperand::CreateImm(EVal, S, E);
  //  }
  //}

  // Finally we have exhausted all options and must declare defeat.
  Error(Parser.getTok().getLoc(), "Unknown operand");
  return true;
}

bool AAPAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                    SMLoc NameLoc, OperandVector &Operands) {
  // First operand is token for instruction
  Operands.push_back(AAPOperand::CreateToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (ParseOperand(Operands))
    return true;

  // Parse until end of statement, consuming commas between operands
  while (getLexer().isNot(AsmToken::EndOfStatement) &&
         getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (ParseOperand(Operands))
      return true;
  }

  // consume end of statement
  return false;
}

bool AAPAsmParser::ParseDirective(AsmToken DirectiveID) { return true; }

extern "C" void LLVMInitializeAAPAsmParser() {
  RegisterMCAsmParser<AAPAsmParser> X(TheAAPTarget);
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#define GET_SUBTARGET_FEATURE_NAME
#include "AAPGenAsmMatcher.inc"
