//===-- AAPInstrInfo.td - Target Description for AAP Target ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the AAP instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

// Call
//def sdt_call         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def sdt_call         : SDTypeProfile<0, -1, [SDTCisVT<1, iPTR>, SDTCisVT<1, i16>]>;
def sdt_ret          : SDTypeProfile<0,  1, [SDTCisVT<0, i16>]>;
def callflag         : SDNode<"AAPISD::CALL", sdt_call,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def retflag          : SDNode<"AAPISD::RET_FLAG", sdt_ret,
                      [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def sdt_callseqstart : SDCallSeqStart<[SDTCisVT<0, i16>]>;
def sdt_callseqend   : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def callseq_start    : SDNode<"ISD::CALLSEQ_START", sdt_callseqstart,
                              [SDNPHasChain, SDNPOutGlue]>;
def callseq_end      : SDNode<"ISD::CALLSEQ_END",   sdt_callseqend,
                              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def sdt_wrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                            SDTCisPtrTy<0>]>;
def aapwrapper : SDNode<"AAPISD::Wrapper", sdt_wrapper>;

def sdt_selectcc     : SDTypeProfile<1, 5, [SDTCisSameAs<0, 3>,
                                            SDTCisSameAs<1, 2>,
                                            SDTCisSameAs<3, 4>,
                                            SDTCisVT<5, i16>]>;
def sdt_brcc         : SDTypeProfile<0, 4, [SDTCisVT<0, i16>,
                                            SDTCisSameAs<1, 2>,
                                            SDTCisVT<3, OtherVT>]>;
def AAPselectcc : SDNode<"AAPISD::SELECT_CC", sdt_selectcc>;
def AAPbrcc     : SDNode<"AAPISD::BR_CC",     sdt_brcc,
                         [SDNPHasChain]>;


// Branch Operands
def brtarget : Operand<OtherVT> {
  let PrintMethod   = "printPCRelImmOperand";
  let EncoderMethod = "encodePCRelImmOperand";
}


// Memory offsets consist of a 6-bit register field in the low bits, and
// a 6-bit immediate offset in the high bits.
def addr_MO6 : ComplexPattern<iPTR, 2, "SelectAddr_MO6", [], []>;


// Memory operands for the various available addressing modes. All memsrc
// operands have the same encoding and only differ in their asm
def memsrcAsmOperand : AsmOperandClass { let Name = "MemSrc"; }
def memsrcPostIncAsmOperand : AsmOperandClass { let Name = "MemSrcPostInc"; }
def memsrcPreDecAsmOperand  : AsmOperandClass { let Name = "MemSrcPreDec"; }

def memsrc : Operand<i16> {
  let EncoderMethod = "encodeMemSrcOperand";
  let DecoderMethod = "decodeMemSrcOperand";
  let PrintMethod = "printMemSrcOperand";
  let ParserMatchClass = memsrcAsmOperand;
  let MIOperandInfo = (ops GR64, i16imm);
}
def memsrc_postinc : Operand<i16> {
  let EncoderMethod = "encodeMemSrcOperand";
  let DecoderMethod = "decodeMemSrcOperand";
  let PrintMethod = "printMemSrcPostIncOperand";
  let ParserMatchClass = memsrcPostIncAsmOperand;
  let MIOperandInfo = (ops GR64, i16imm);
}
def memsrc_predec : Operand<i16> {
  let EncoderMethod = "encodeMemSrcOperand";
  let DecoderMethod = "decodeMemSrcOperand";
  let PrintMethod = "printMemSrcPreDecOperand";
  let ParserMatchClass = memsrcPreDecAsmOperand;
  let MIOperandInfo = (ops GR64, i16imm);
}


def imm3 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 8;
}]> {
  let EncoderMethod = "encodeImm3Operand";
}
def imm6 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 64;
}]> {
  let EncoderMethod = "encodeImm6Operand";
}
def imm9 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 512;
}]> {
  let EncoderMethod = "encodeImm9Operand";
}
def imm10 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 1024;
}]> {
  let EncoderMethod = "encodeImm10Operand";
}
def imm12 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 4094;
}]> {
  let EncoderMethod = "encodeImm12Operand";
}
def imm16 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm < 65536;
}]> {
  let EncoderMethod = "encodeImm16Operand";
}


//===----------------------------------------------------------------------===//
// MOV Operations
//===----------------------------------------------------------------------===//

def MOV_r : Inst_rr
  <0x0, 0x9, (outs GR64:$rD), (ins GR64:$rA), "mov\t$rD, $rA", []>;

def MOV_r_short : Inst_rr_short
  <0x0, 0x9, (outs GR8:$rD), (ins GR8:$rA), "mov\t$rD, $rA", []>;

def MOV_i16 : Inst_r_i16
  <0x0, 0xf, (outs GR64:$rD), (ins imm16:$imm), "mov\t$rD, $imm",
    [(set GR64:$rD, (i16 imm:$imm))]>;

def MOV_i6_short : Inst_r_i6_short
  <0x0, 0xf, (outs GR8:$rD), (ins imm6:$imm), "mov\t$rD, $imm",
    [(set GR8:$rD, (i16 imm:$imm))]>;


//===----------------------------------------------------------------------===//
// ALU/Logical Operations
//===----------------------------------------------------------------------===//

def NOP : Inst_r_i12
  <0x0, 0x0, (outs), (ins GR64:$rD, imm12:$imm), "nop\t$rD, $imm", []>;

def NOP_short : Inst_r_i6_short
  <0x0, 0x0, (outs), (ins GR8:$rD, imm6:$imm), "nop\t$rD, $imm", []>;


// ALU ops with register operands
multiclass ALU_r<bits<8> opcode, string opname, SDNode OpNode> {
  def _r : Inst_rrr
    <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
      !strconcat(opname, "\t$rD, $rA, $rB"),
      [(set GR64:$rD, (OpNode GR64:$rA, GR64:$rB))]>;

  def _r_short : Inst_rrr_short
    <0x0, opcode{3-0}, (outs GR8:$rD), (ins GR8:$rA, GR8:$rB),
      !strconcat(opname, "\t$rD, $rA, $rB"),
      [(set GR8:$rD, (OpNode GR8:$rA, GR8:$rB))]>;
}


let isCommutable = 1 in {
let Defs = [PSW] in
  defm ADD : ALU_r<0x1, "add", add>;
defm AND : ALU_r<0x3, "and", and>;
defm OR  : ALU_r<0x4, "or",  or>;
defm XOR : ALU_r<0x5, "xor", xor>;
}

let Defs = [PSW] in
  defm SUB : ALU_r<0x2, "sub", sub>;
defm ASR : ALU_r<0x6, "asr", sra>;
defm LSL : ALU_r<0x7, "lsl", shl>;
defm LSR : ALU_r<0x8, "lsr", srl>;


let Uses = [PSW], Defs = [PSW] in {
def ADDC_r : Inst_rrr
  <0x0, 0x11, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
    "addc\t$rD, $rA, $rB",
    [(set GR64:$rD, (adde GR64:$rA, GR64:$rB))]>;

def SUBC_r : Inst_rrr
  <0x0, 0x12, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
    "subc\t$rD, $rA, $rD",
    [(set GR64:$rD, (sube GR64:$rA, GR64:$rB))]>;
}


// ALU ops with immediates
let Defs = [PSW] in {
def ADD_i10 : Inst_rr_i10
  <0x0, 0xa, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
    "add\t$rD, $rA, $imm",
    [(set GR64:$rD, (add GR64:$rA, (i16 imm:$imm)))]>;

def ADD_i3_short : Inst_rr_i3_short
  <0x0, 0xa, (outs GR8:$rD), (ins GR8:$rA, imm3:$imm),
    "add\t$rD, $rA, $imm",
    [(set GR8:$rD, (add GR8:$rA, (i16 imm:$imm)))]>;

def SUB_i10 : Inst_rr_i10
  <0x0, 0xb, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
    "sub\t$rD, $rA, $imm",
    [(set GR64:$rD, (sub GR64:$rA, (i16 imm:$imm)))]>;

def SUB_i3_short : Inst_rr_i3_short
  <0x0, 0xb, (outs GR8:$rD), (ins GR8:$rA, imm3:$imm),
    "sub\t$rD, $rA, $imm",
    [(set GR8:$rD, (sub GR8:$rA, (i16 imm:$imm)))]>;
}


multiclass SHIFT_i<bits<8> opcode, string opname, SDNode OpNode> {
  def _i6 : Inst_rr_i6
    <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, imm6:$imm),
      !strconcat(opname, "\t$rD, $rA, $imm"),
      [(set GR64:$rD, (OpNode GR64:$rA, (i16 imm:$imm)))]>;

  def _i3_short : Inst_rr_i3_short
    <0x0, opcode{3-0}, (outs GR8:$rD), (ins GR8:$rA, imm3:$imm),
      !strconcat(opname, "\t$rD, $rA, $imm"),
      [(set GR8:$rD, (OpNode GR8:$rA, (i16 imm:$imm)))]>;
}
defm ASR : SHIFT_i<0xc, "asr", sra>;
defm LSL : SHIFT_i<0xd, "lsl", shl>;
defm LSR : SHIFT_i<0xe, "lsr", srl>;


// Logical operations with immediate
class LOG_i9<bits<5> opcode, string opname, SDNode OpNode>
    : Inst_rr_i9
      <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, imm9:$imm),
        !strconcat(opname, "\t$rD, $rA, $imm"),
        [(set GR64:$rD, (OpNode GR64:$rA, (i16 imm:$imm)))]>;

def AND_i9 : LOG_i9<0x13, "and", and>;
def OR_i9  : LOG_i9<0x14, "or",  or>;
def XOR_i9 : LOG_i9<0x15, "xor", xor>;


//===----------------------------------------------------------------------===//
// Load/Store Operations
//===----------------------------------------------------------------------===//

// TODO: It may be better for the LOAD/STORE classes to be in the formats .td
// TODO: There's quite a lot of duplication here as the Load/Store classes
// end up defining a lot of the fields themselves.

class LOAD<bits<8> opcode, string opname, dag outs, dag ins>
    : InstAAP
      <0x1, opcode, outs, ins, !strconcat(opname, "\t$rD, [$src]"), []> {
  bits<6> rD;
  bits<12> src;

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};

  let Inst{5-3} = src{2-0};
  let Inst{21-19} = src{5-3};
  let Inst{2-0} = src{8-6};
  let Inst{18-16} = src{11-9};
}

class LOAD_short<bits<4> opcode, string opname, dag outs, dag ins>
    : InstAAP_short
      <0x1, opcode, outs, ins, !strconcat(opname, "\t$rD, [$src]"), []> {
  bits<3> rD;
  bits<6> src;

  let Inst{8-6} = rD;
  let Inst{5-3} = src{2-0};
  let Inst{2-0} = src{5-3};
}

class STORE<bits<8> opcode, string opname, dag outs, dag ins>
    : InstAAP
      <0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"), []> {
  bits<12> dst;
  bits<6> rA;

  let Inst{8-6} = dst{2-0};
  let Inst{24-22} = dst{5-3};
  let Inst{2-0} = dst{8-6};
  let Inst{18-16} = dst{11-9};

  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
}

class STORE_short<bits<4> opcode, string opname, dag outs, dag ins>
    : InstAAP_short
      <0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"), []> {
  bits<6> dst;
  bits<3> rA;

  let Inst{8-6} = dst{2-0};
  let Inst{2-0} = dst{5-3};
  let Inst{5-3} = rA;
}


let mayLoad = 1 in {
def LDB : LOAD<0x0,  "ldb", (outs GR64:$rD), (ins memsrc:$src)>;
def LDW : LOAD<0x4,  "ldw", (outs GR64:$rD), (ins memsrc:$src)>;
def LDD : LOAD<0x40, "ldd", (outs GR64:$rD), (ins memsrc:$src)>;
def LDB_postinc : LOAD<0x1,  "ldb", (outs GR64:$rD), (ins memsrc_postinc:$src)>;
def LDW_postinc : LOAD<0x5,  "ldw", (outs GR64:$rD), (ins memsrc_postinc:$src)>;
def LDD_postinc : LOAD<0x41, "ldd", (outs GR64:$rD), (ins memsrc_postinc:$src)>;
def LDB_predec  : LOAD<0x2,  "ldb", (outs GR64:$rD), (ins memsrc_predec:$src)>;
def LDW_predec  : LOAD<0x6,  "ldw", (outs GR64:$rD), (ins memsrc_predec:$src)>;
def LDD_predec  : LOAD<0x42, "ldd", (outs GR64:$rD), (ins memsrc_predec:$src)>;

def LDB_short : LOAD_short<0x0, "ldb", (outs GR8:$rD), (ins memsrc:$src)>;
def LDW_short : LOAD_short<0x4, "ldw", (outs GR8:$rD), (ins memsrc:$src)>;
def LDB_postinc_short : LOAD_short<0x1, "ldb", (outs GR8:$rD), (ins memsrc_postinc:$src)>;
def LDW_postinc_short : LOAD_short<0x5, "ldw", (outs GR8:$rD), (ins memsrc_postinc:$src)>;
def LDB_predec_short  : LOAD_short<0x2, "ldb", (outs GR8:$rD), (ins memsrc_predec:$src)>;
def LDW_predec_short  : LOAD_short<0x6, "ldw", (outs GR8:$rD), (ins memsrc_predec:$src)>;
}

// Load patterns
def : Pat<(i16 (load addr_MO6:$src)), (LDW addr_MO6:$src)>;

let mayStore = 1 in {
def STB : STORE<0x8,  "stb", (outs), (ins memsrc:$dst, GR64:$rA)>;
def STW : STORE<0xc,  "stw", (outs), (ins memsrc:$dst, GR64:$rA)>;
def STD : STORE<0x48, "std", (outs), (ins memsrc:$dst, GR64:$rA)>;
def STB_postinc : STORE<0x9,  "stb", (outs), (ins memsrc_postinc:$dst, GR64:$rA)>;
def STW_postinc : STORE<0xd,  "stw", (outs), (ins memsrc_postinc:$dst, GR64:$rA)>;
def STD_postinc : STORE<0x49, "std", (outs), (ins memsrc_postinc:$dst, GR64:$rA)>;
def STB_predec  : STORE<0xa,  "stb", (outs), (ins memsrc_predec:$dst, GR64:$rA)>;
def STW_predec  : STORE<0xe,  "stw", (outs), (ins memsrc_predec:$dst, GR64:$rA)>;
def STD_predec  : STORE<0x4a, "std", (outs), (ins memsrc_predec:$dst, GR64:$rA)>;

def STB_short : STORE_short<0x8, "stb", (outs), (ins memsrc:$dst, GR8:$rA)>;
def STW_short : STORE_short<0xc, "stw", (outs), (ins memsrc:$dst, GR8:$rA)>;
def STB_postinc_short : STORE_short<0x9, "stb", (outs), (ins memsrc_postinc:$dst, GR8:$rA)>;
def STW_postinc_short : STORE_short<0xd, "stw", (outs), (ins memsrc_postinc:$dst, GR8:$rA)>;
def STB_predec_short  : STORE_short<0xa, "stb", (outs), (ins memsrc_predec:$dst, GR8:$rA)>;
def STW_predec_short  : STORE_short<0xe, "stw", (outs), (ins memsrc_predec:$dst, GR8:$rA)>;
}

// Store patterns
def : Pat<(store GR64:$rA, addr_MO6:$dst), (STW addr_MO6:$dst, GR64:$rA)>;


//===----------------------------------------------------------------------===//
// Branch Operations
//===----------------------------------------------------------------------===//

multiclass BRCC<bits<4> cc, string opname> {
  def _ : Inst_i10_rr
    <0x2, cc, (outs), (ins brtarget:$imm, GR64:$rA, GR64:$rB),
      !strconcat(opname, "\t$imm, $rA, $rB"), []>;

  def _short : Inst_i3_rr_short
    <0x2, cc, (outs), (ins brtarget:$imm, GR8:$rA, GR8:$rB),
      !strconcat(opname, "\t$imm, $rA, $rB"), []>;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
  defm BEQ   : BRCC<0x2, "beq">;
  defm BNE   : BRCC<0x3, "bne">;
  defm BLTS  : BRCC<0x4, "blts">;
  defm BGTS  : BRCC<0x5, "bgts">;
  defm BLTU  : BRCC<0x6, "bltu">;
  defm BGTU  : BRCC<0x7, "bgtu">;

  def BRA : Inst_i18
    <0x2, 0x0, (outs), (ins brtarget:$imm), "bra\t$imm", [(br bb:$imm)]>;

  def BRA_short : Inst_i9_short
    <0x2, 0x0, (outs), (ins brtarget:$imm), "bra\t$imm", []>;
}

// Implements the BR_CC DAG node. Expanded into one of the above branches.
let isBranch = 1, isTerminator = 1, isBarrier = 1, usesCustomInserter = 1 in {
  def BR_CC : Pseudo
    <(outs), (ins i16imm:$cc, GR64:$lhs, GR64:$rhs, brtarget:$target),
      "#BR_CC", [(AAPbrcc imm:$cc, GR64:$lhs, GR64:$rhs, bb:$target)]>;

  def BR_CC_short : Pseudo
    <(outs), (ins i16imm:$cc, GR8:$lhs, GR8:$rhs, brtarget:$target),
      "#BR_CC_short", []>;
}

// Implements the SELECT_CC DAG node. Expanded into a branch sequence
let usesCustomInserter = 1 in {
  def SELECT_CC : Pseudo
    <(outs GR64:$dst),
      (ins GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, i16imm:$cc),
      "#SELECT_CC",
      [(set GR64:$dst,
        (AAPselectcc GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, imm:$cc))]>;
}


//===----------------------------------------------------------------------===//
// Calls and returns
//===----------------------------------------------------------------------===//
//FIXME: Mark as clobbering all non-callee saved regs
//TODO: Add other conditional branches
let isCall = 1,
  Uses = [R1],
  Defs = [R0, R2,  R3,  R4,  R5,  R6,  R7,  R8,  R9, R10, R11, R12,
          R13, R14, R15, R16, R17, R18, R19] in {
def BAL_short : Inst_i6_r_short
  <0x2, 0x8, (outs), (ins i16imm:$imm, GR8:$rA), "bal\t$imm, $rA", []>;

def BAL : Inst_i16_r
  <0x2, 0x8, (outs), (ins i16imm:$imm, GR64:$rA), "bal\t$imm, $rA", []>;
}

// Calls are encoded as a branch through the link register
def : Pat<(callflag (i16 imm:$imm), GR64:$rA), (BAL imm:$imm, GR64:$rA)>;


let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
def JMP : Inst_r<0x2, 0x9, (outs), (ins GR64:$rD), "jmp\t$rD", []>;
def JMP_short : Inst_r_short<0x2, 0x9, (outs), (ins GR8:$rD), "jmp\t$rD", []>;
}

// Returns are encoded as a jump through the link register
def : Pat<(retflag GR64:$rD), (JMP GR64:$rD)>;


// Stack adjustment
let Defs = [R1], Uses = [R1] in {
def ADJCALLSTACKDOWN : Pseudo
  <(outs), (ins i16imm:$amt), "#ADJCALLSTACKDOWN",
    [(callseq_start timm:$amt)]>;

def ADJCALLSTACKUP : Pseudo
  <(outs), (ins i16imm:$amt1, i16imm:$amt2), "#ADJCALLSTACKUP",
    [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// Call patterns
def : Pat<(callflag (i16 tglobaladdr:$dst), GR64:$rA),  (BAL tglobaladdr:$dst, GR64:$rA)>;
def : Pat<(callflag (i16 texternalsym:$dst), GR64:$rA), (BAL texternalsym:$dst, GR64:$rA)>;


//===----------------------------------------------------------------------===//
// Peephole Patterns
//===----------------------------------------------------------------------===//
def : Pat<(i16 (aapwrapper tglobaladdr:$dst)), (MOV_r tglobaladdr:$dst)>;
def : Pat<(i16 (aapwrapper texternalsym:$dst)), (MOV_r texternalsym:$dst)>;
def : Pat<(i16 (aapwrapper tblockaddress:$dst)), (MOV_r tblockaddress:$dst)>;
