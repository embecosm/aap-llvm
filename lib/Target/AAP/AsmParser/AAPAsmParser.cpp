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
  const MCRegisterInfo *MRI;

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

    // Cache the machine register info for later
    MRI = Parser.getContext().getRegisterInfo();
  }

  unsigned checkTargetMatchPredicate(MCInst &Inst) override {
    // Check instructions with memsrc operands here, as they cannot be
    // checked through the instruction match classes.

    // If this is a short load/store instruction, then we must check that
    // its register operands are all in the GR8 reg class.
    const MCRegisterClass& MRC = MRI->getRegClass(AAP::GR8RegClassID);

    switch (Inst.getOpcode()) {
    default:
      return Match_Success;
    case AAP::LDB_short:
    case AAP::LDW_short:
    case AAP::LDB_postinc_short:
    case AAP::LDW_postinc_short:
    case AAP::LDB_predec_short:
    case AAP::LDW_predec_short:
    case AAP::STB_short:
    case AAP::STW_short:
    case AAP::STB_postinc_short:
    case AAP::STW_postinc_short:
    case AAP::STB_predec_short:
    case AAP::STW_predec_short:
      break;
    }

    for (MCInst::iterator I = Inst.begin(); I != Inst.end(); ++I) {
      if (I->isReg() && !MRC.contains(I->getReg())) {
        return Match_InvalidOperand;
      }
    }
    return Match_Success;
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
      bool WithPreDec;
      unsigned RegNum;
      bool WithPostInc;
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


  // If an MCExpr is a constant immediate, check whether it is in the provided
  // inclusive range. If we are checking for an immediate, then arbitrary
  // expressions are also allowed.
  static bool isImmInRange(const MCExpr* Imm, int64_t Min, int64_t Max) {
    if (Imm->getKind() != MCExpr::Constant) {
      return true;    // allow arbitrary expressions
    }
    int64_t Res;
    Imm->EvaluateAsAbsolute(Res);
    return (Res >= Min) && (Res <= Max);
  }

  static bool isConstInRange(const MCExpr* Imm, int64_t Min, int64_t Max) {
    if (Imm->getKind() != MCExpr::Constant) {
      return false;   // only allow constants
    }
    int64_t Res;
    Imm->EvaluateAsAbsolute(Res);
    return (Res >= Min) && (Res <= Max);
  }

  static bool isConst3(const MCExpr* I) { return isConstInRange(I, 0, 7); }
  static bool isConst6(const MCExpr* I) { return isConstInRange(I, 0, 63); }
  static bool isImm6(const MCExpr* I)  { return isImmInRange(I, 0, 63); }
  static bool isImm9(const MCExpr* I)  { return isImmInRange(I, 0, 511); }
  static bool isImm10(const MCExpr* I) { return isImmInRange(I, 0, 1023); }
  static bool isImm12(const MCExpr* I) { return isImmInRange(I, 0, 4095); }
  static bool isImm16(const MCExpr* I) { return isImmInRange(I, -32768, 65535); }
  static bool isOff10(const MCExpr* I) { return isImmInRange(I, -512, 511); }


  // Functions for testing operand type
  bool isReg() const { return Kind == Register; }
  bool isImm() const { return Kind == Immediate; }

  bool isConst3() const { return isImm() && isConst3(getImm()); }
  bool isConst6() const { return isImm() && isConst6(getImm()); }
  bool isImm6()  const { return isImm() && isImm6(getImm()); }
  bool isImm9()  const { return isImm() && isImm9(getImm()); }
  bool isImm10() const { return isImm() && isImm10(getImm()); }
  bool isImm12() const { return isImm() && isImm12(getImm()); }
  bool isImm16() const { return isImm() && isImm16(getImm()); }
  bool isOff10() const { return isImm() && isOff10(getImm()); }

  bool isToken() const { return Kind == Token; }
  bool isMem() const { return false; }

  // Check that the immediate fits in an imm6
  bool isMemSrc10() const {
    if (Kind != MemSrc || Mem.WithPreDec || Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR64 class
    return isOff10(getMemSrcImm());
  }
  bool isMemSrc10PostInc() const {
    if (Kind != MemSrc)   { return false; }
    if (Mem.WithPreDec)   { return false; }
    if (!Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR64 class
    return isOff10(getMemSrcImm());
  }
  bool isMemSrc10PreDec() const {
    if (Kind != MemSrc)  { return false; }
    if (!Mem.WithPreDec) { return false; }
    if (Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR64 class
    return isOff10(getMemSrcImm());
  }

  // Check that the immediate fits in an const3. A memsrc3 operand may only
  // have a constant in its immediate field.
  bool isMemSrc3() const {
    if (Kind != MemSrc || Mem.WithPreDec || Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR8 class
    return isConst3(getMemSrcImm());
  }
  bool isMemSrc3PostInc() const {
    if (Kind != MemSrc)   { return false; }
    if (Mem.WithPreDec)   { return false; }
    if (!Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR8 class
    return isConst3(getMemSrcImm());
  }
  bool isMemSrc3PreDec() const {
    if (Kind != MemSrc)  { return false; }
    if (!Mem.WithPreDec) { return false; }
    if (Mem.WithPostInc) { return false; }
    // AAPAsmParser::checkTargetMatchPredicate checks that the register is
    // is in the GR8 class
    return isConst3(getMemSrcImm());
  }

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
  void addMemSrc10Operands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
  }
  void addMemSrc10PostIncOperands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
  }
  void addMemSrc10PreDecOperands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
  }
  void addMemSrc3Operands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
  }
  void addMemSrc3PostIncOperands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
  }
  void addMemSrc3PreDecOperands(MCInst &Inst, unsigned N) const {
    addMemSrcOperands(Inst, N);
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
  CreateMemSrc(unsigned RegNo, const MCExpr *Offset,
               bool WithPreDec, bool WithPostInc, SMLoc S, SMLoc E) {
    auto Op = make_unique<AAPOperand>(MemSrc);
    Op->Mem.WithPreDec = WithPreDec;
    Op->Mem.RegNum = RegNo;
    Op->Mem.WithPostInc = WithPostInc;
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

  // Registers have '$' prefix
  if (getLexer().getKind() != AsmToken::Dollar) {
    return nullptr;
  }
  getLexer().Lex();

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
}

std::unique_ptr<AAPOperand> AAPAsmParser::ParseImmediate() {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  const MCExpr *EVal;
  if (!getParser().parseExpression(EVal, E)) {
    return AAPOperand::CreateImm(EVal, S, E);
  }
  return nullptr;
}

std::unique_ptr<AAPOperand> AAPAsmParser::ParseMemSrc() {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  bool WithPreDec = false;
  bool WithPostInc = false;

  // First check for a '-', signifying a predecrement
  if (getLexer().getKind() == AsmToken::Minus) {
    getLexer().Lex();
    WithPreDec = true;
  }

  unsigned RegNo;
  std::unique_ptr<AAPOperand> RegOp = ParseRegister(RegNo);
  if (!RegOp) {
    Error(Parser.getTok().getLoc(), "Missing register in memsrc operand");
    return nullptr;
  }
  RegNo = RegOp->getReg();

  // Check for a '+' signifying a post increment
  if (getLexer().getKind() == AsmToken::Plus) {
    getLexer().Lex();
    WithPostInc = true;
  }

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

  return AAPOperand::CreateMemSrc(RegNo, ImmVal, WithPreDec, WithPostInc, S, E);
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

  // If we see a '[', that implies a memsrc follows
  if (getLexer().getKind() == AsmToken::LBrac) {
    SMLoc S = Parser.getTok().getLoc();
    Operands.push_back(AAPOperand::CreateToken("[", S));
    Parser.Lex(); // Eat '['

    Op = ParseMemSrc();

    if (Op) {
      Operands.push_back(std::move(Op));

      S = Parser.getTok().getLoc();
      if (getLexer().getKind() == AsmToken::RBrac) {
        Operands.push_back(AAPOperand::CreateToken("]", S));
        Parser.Lex(); // Eat ']'
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
