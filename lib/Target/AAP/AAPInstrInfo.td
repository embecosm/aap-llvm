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


// Immediate operands. Smaller operands are subclasses of larger ones so they
// are correctly prioritized when disassembling.
//
// Currently we have no custom 'signed' immediates, but it will likely
// be needed in the future to handle branches properly.
//
// A constant immediate is an immediate which cannot be an expression, only
// an absolute value.
let RenderMethod = "addImmOperands" in {
  def imm16AsmOperand : AsmOperandClass {
    let Name = "Imm16"; let SuperClasses = [ImmAsmOperand];
  }
  def imm12AsmOperand : AsmOperandClass {
    let Name = "Imm12"; let SuperClasses = [imm16AsmOperand];
  }
  def imm10AsmOperand : AsmOperandClass {
    let Name = "Imm10"; let SuperClasses = [imm12AsmOperand];
  }
  def imm9AsmOperand  : AsmOperandClass {
    let Name = "Imm9"; let SuperClasses = [imm10AsmOperand];
  }
  def const6AsmOperand  : AsmOperandClass {
    let Name = "Const6"; let SuperClasses = [imm9AsmOperand];
  }
  def imm6AsmOperand  : AsmOperandClass {
    let Name = "Imm6"; let SuperClasses = [const6AsmOperand];
  }
  def const3AsmOperand  : AsmOperandClass {
    let Name = "Const3"; let SuperClasses = [imm6AsmOperand];
  }

  // Offsets are signed immediates used for load and store operands
  def off10AsmOperand : AsmOperandClass {
    let Name = "Off10"; let SuperClasses = [ImmAsmOperand];
  }
  def off3AsmOperand : AsmOperandClass {
    let Name = "Off3"; let SuperClasses = [off10AsmOperand];
  }

  // Shift operands are unsigned immediates or constants who range
  // begins at 1, rather than 0. The encoding is done with the shift amount
  // -1.
  def shiftImm6AsmOperand : AsmOperandClass {
    let Name = "ShiftImm6"; let SuperClasses = [imm9AsmOperand];
  }
  def shiftConst3AsmOperand : AsmOperandClass {
    let Name = "ShiftConst3"; let SuperClasses = [shiftImm6AsmOperand];
  }
}

def imm16 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= -32768 && Imm <= 65535;
}]> {
  let EncoderMethod = "encodeImm16";
  let ParserMatchClass = imm16AsmOperand;
}
def imm12 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 4095;
}]> {
  let EncoderMethod = "encodeImm12";
  let ParserMatchClass = imm12AsmOperand;
}
def imm10 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 1023;
}]> {
  let EncoderMethod = "encodeImm10";
  let ParserMatchClass = imm10AsmOperand;
}
def imm9 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 511;
}]> {
  let EncoderMethod = "encodeImm9";
  let ParserMatchClass = imm9AsmOperand;
}
def const6 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 63;
}]> {
  let EncoderMethod = "encodeImm6";
  let ParserMatchClass = const6AsmOperand;
}
def imm6 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 63;
}]> {
  let EncoderMethod = "encodeImm6";
  let ParserMatchClass = imm6AsmOperand;
}
def const3 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 0 && Imm <= 7;
}]> {
  let EncoderMethod = "encodeImm3";
  let ParserMatchClass = const3AsmOperand;
}

// Offset operands
def off10 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= -512 && Imm <= 511;
}]> {
  let EncoderMethod = "encodeOff10";
  let ParserMatchClass = off10AsmOperand;
}
def off3 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= -4 && Imm <= 3;
}]> {
  let EncoderMethod = "encodeOff3";
  let ParserMatchClass = off3AsmOperand;
}

// Shift operands, these have custom encoding/decoding to handle the bias
// applied to the value.
def shift_imm6 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 1 && Imm <= 64;
}]> {
  let EncoderMethod = "encodeShiftImm6";
  let DecoderMethod = "decodeShiftOperand";
  let ParserMatchClass = shiftImm6AsmOperand;
}
def shift_const3 : Operand<i16>, ImmLeaf<i16, [{
  return Imm >= 1 && Imm <= 8;
}]> {
  let EncoderMethod = "encodeShiftConst3";
  let DecoderMethod = "decodeShiftOperand";
  let ParserMatchClass = shiftConst3AsmOperand;
}


// Memory offsets consist of a 3 or 10 bit offset and a register operand
def addr_MO3  : ComplexPattern<iPTR, 2, "SelectAddr_MO3",  [], []>;
def addr_MO10 : ComplexPattern<iPTR, 2, "SelectAddr_MO10", [], []>;


// Memsrc operand encodings consist of a 16-bit immediate field in the low
// bits, and a 3 or 6 bit register field in the high bits. The encoding
// is exactly the same for predecrement and postincrement address mode.
def memsrc10AsmOperand : AsmOperandClass {
  let Name = "MemSrc10";
}
def memsrc10PostIncAsmOperand : AsmOperandClass {
  let Name = "MemSrc10PostInc";
}
def memsrc10PreDecAsmOperand  : AsmOperandClass {
  let Name = "MemSrc10PreDec";
}

// short memsrc operands subclass the longer ones, as this means they will
// be prioritized when matching assembly.
def memsrc3AsmOperand : AsmOperandClass {
  let Name = "MemSrc3";
  let SuperClasses = [memsrc10AsmOperand];
}
def memsrc3PostIncAsmOperand : AsmOperandClass {
  let Name = "MemSrc3PostInc";
  let SuperClasses = [memsrc10PostIncAsmOperand];
}
def memsrc3PreDecAsmOperand  : AsmOperandClass {
  let Name = "MemSrc3PreDec";
  let SuperClasses = [memsrc10PreDecAsmOperand];
}

let EncoderMethod = "encodeMemSrcOperand",
    DecoderMethod = "decodeMemSrcOperand" in {
  def memsrc10 : Operand<i16> {
    let PrintMethod = "printMemSrcOperand";
    let ParserMatchClass = memsrc10AsmOperand;
    let MIOperandInfo = (ops GR64, off10);
  }
  def memsrc10_postinc : Operand<i16> {
    let PrintMethod = "printMemSrcPostIncOperand";
    let ParserMatchClass = memsrc10PostIncAsmOperand;
    let MIOperandInfo = (ops GR64, off10);
  }
  def memsrc10_predec : Operand<i16> {
    let PrintMethod = "printMemSrcPreDecOperand";
    let ParserMatchClass = memsrc10PreDecAsmOperand;
    let MIOperandInfo = (ops GR64, off10);
  }
  def memsrc3 : Operand<i16> {
    let PrintMethod = "printMemSrcOperand";
    let ParserMatchClass = memsrc3AsmOperand;
    let MIOperandInfo = (ops GR8, off3);
  }
  def memsrc3_postinc : Operand<i16> {
    let PrintMethod = "printMemSrcPostIncOperand";
    let ParserMatchClass = memsrc3PostIncAsmOperand;
    let MIOperandInfo = (ops GR8, off3);
  }
  def memsrc3_predec : Operand<i16> {
    let PrintMethod = "printMemSrcPreDecOperand";
    let ParserMatchClass = memsrc3PreDecAsmOperand;
    let MIOperandInfo = (ops GR8, off3);
  }
}


//===----------------------------------------------------------------------===//
// MOV Operations
//===----------------------------------------------------------------------===//

def MOV_r : Inst_rrr
  <0x0, 0x9, (outs GR64:$rD), (ins GR64:$rA), "mov\t$rD, $rA", []> {
    let rB = 0;
  }

// Select over MOV_r
let AddedComplexity = 1 in
  def MOV_r_short : Inst_rrr_short
    <0x0, 0x9, (outs GR8:$rD), (ins GR8:$rA), "mov\t$rD, $rA", []> {
    let rB = 0;
  }

def MOVI_i16 : Inst_r_i16
  <0x0, 0xf, (outs GR64:$rD), (ins imm16:$imm), "movi\t$rD, $imm",
    [(set GR64:$rD, (i16 imm16:$imm))]>;

// Select over MOVI_i16
let AddedComplexity = 1 in
  def MOVI_i6_short : Inst_r_i6_short
    <0x0, 0xf, (outs GR8:$rD), (ins const6:$imm), "movi\t$rD, $imm",
      [(set GR8:$rD, (i16 const6:$imm))]>;


//===----------------------------------------------------------------------===//
// ALU/Logical Operations
//===----------------------------------------------------------------------===//

def NOP : Inst_r_i12
  <0x0, 0x0, (outs), (ins GR64:$rD, imm12:$imm), "nop\t$rD, $imm", []>;

def NOP_short : Inst_r_i6_short
  <0x0, 0x0, (outs), (ins GR8:$rD, const6:$imm), "nop\t$rD, $imm", []>;


// ALU ops with register operands
multiclass ALU_r<bits<8> opcode, string opname, SDNode OpNode> {
  def _r : Inst_rrr
    <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
      !strconcat(opname, "\t$rD, $rA, $rB"),
      [(set GR64:$rD, (OpNode GR64:$rA, GR64:$rB))]>;

  // Select over ALU_r
  let AddedComplexity = 1 in
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
    "subc\t$rD, $rA, $rB",
    [(set GR64:$rD, (sube GR64:$rA, GR64:$rB))]>;
}


// ALU ops with immediates
let Defs = [PSW] in {

// LEA Pseudo operation, expanded to ADD_i10 or SUB_i10 during frame index
// elimination depending on the sign of the immediate.
def LEA : Pseudo<(outs GR64:$rD), (ins GR64:$rA, imm16:$imm), "#LEA", []>;

// Select over MOVI_i16 + ADD_r_short
let AddedComplexity = 2 in
  def ADDI_i10 : Inst_rr_i10
    <0x0, 0xa, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
      "addi\t$rD, $rA, $imm",
      [(set GR64:$rD, (add GR64:$rA, (i16 imm10:$imm)))]>;

// Select over ADDI_i10
let AddedComplexity = 3 in
  def ADDI_i3_short : Inst_rr_i3_short
    <0x0, 0xa, (outs GR8:$rD), (ins GR8:$rA, const3:$imm),
      "addi\t$rD, $rA, $imm",
      [(set GR8:$rD, (add GR8:$rA, (i16 const3:$imm)))]>;

// Select over MOVI_i16 + SUB_r_short
let AddedComplexity = 2 in
  def SUBI_i10 : Inst_rr_i10
    <0x0, 0xb, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
      "subi\t$rD, $rA, $imm",
      [(set GR64:$rD, (sub GR64:$rA, (i16 imm10:$imm)))]>;

// Select over SUBI_i10
let AddedComplexity = 3 in
  def SUBI_i3_short : Inst_rr_i3_short
    <0x0, 0xb, (outs GR8:$rD), (ins GR8:$rA, const3:$imm),
      "subi\t$rD, $rA, $imm",
      [(set GR8:$rD, (sub GR8:$rA, (i16 const3:$imm)))]>;

} // end of Defs


multiclass SHIFT_i<bits<8> opcode, string opname, SDNode OpNode> {
  // Select over MOVI_i16 + SHIFT_r_short
  let AddedComplexity = 2 in
    def _i6 : Inst_rr_i6
      <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, shift_imm6:$imm),
        !strconcat(opname, "\t$rD, $rA, $imm"),
        [(set GR64:$rD, (OpNode GR64:$rA, (i16 shift_imm6:$imm)))]>;

  // Select over SHIFTI_i6
  let AddedComplexity = 3 in
    def _i3_short : Inst_rr_i3_short
      <0x0, opcode{3-0}, (outs GR8:$rD), (ins GR8:$rA, shift_const3:$imm),
        !strconcat(opname, "\t$rD, $rA, $imm"),
        [(set GR8:$rD, (OpNode GR8:$rA, (i16 shift_const3:$imm)))]>;
}
defm ASRI : SHIFT_i<0xc, "asri", sra>;
defm LSLI : SHIFT_i<0xd, "lsli", shl>;
defm LSRI : SHIFT_i<0xe, "lsri", srl>;


// Logical operations with immediate
// Select over MOVI_i16 + LOG_r_short
let AddedComplexity = 2 in {
class LOG_i9<bits<5> opcode, string opname, SDNode OpNode>
    : Inst_rr_i9
      <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, imm9:$imm),
        !strconcat(opname, "\t$rD, $rA, $imm"),
        [(set GR64:$rD, (OpNode GR64:$rA, (i16 imm9:$imm)))]>;
}

def ANDI_i9 : LOG_i9<0x13, "andi", and>;
def ORI_i9  : LOG_i9<0x14, "ori",  or>;
def XORI_i9 : LOG_i9<0x15, "xori", xor>;


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
  bits<22> src; // 6-bit reg field, 16 bit imm field

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};

  let Inst{5-3} = src{18-16};
  let Inst{21-19} = src{21-19};
  let Inst{2-0} = src{2-0};
  let Inst{18-16} = src{5-3};
  let Inst{28-25} = src{9-6}; // reuse second word opcode field for offset
}

class LOAD_short<bits<4> opcode, string opname, dag outs, dag ins>
    : InstAAP_short
      <0x1, opcode, outs, ins, !strconcat(opname, "\t$rD, [$src]"), []> {
  bits<3> rD;
  bits<19> src; // 3-bit reg field, 16 bit imm field

  let Inst{8-6} = rD;
  let Inst{5-3} = src{18-16};
  let Inst{2-0} = src{2-0};
}

class STORE<bits<8> opcode, string opname, dag outs, dag ins>
    : InstAAP
      <0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"), []> {
  bits<22> dst; // 6-bit reg field, 16 bit imm field
  bits<6> rA;

  let Inst{8-6} = dst{18-16};
  let Inst{24-22} = dst{21-19};
  let Inst{2-0} = dst{2-0};
  let Inst{18-16} = dst{5-3};
  let Inst{28-25} = dst{9-6}; // reuse second word opcode field for offset

  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
}

class STORE_short<bits<4> opcode, string opname, dag outs, dag ins>
    : InstAAP_short
      <0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"), []> {
  bits<19> dst; // 3-bit reg field, 16 bit imm field
  bits<3> rA;

  let Inst{8-6} = dst{18-16};
  let Inst{2-0} = dst{2-0};
  let Inst{5-3} = rA;
}


// Short instructions use the memsrc3 operand type, which uses a different
// fixup when encoding and a different matching class when parsing assembly.
let mayLoad = 1 in {
def LDB : LOAD<0x0,  "ldb", (outs GR64:$rD), (ins memsrc10:$src)>;
def LDW : LOAD<0x4,  "ldw", (outs GR64:$rD), (ins memsrc10:$src)>;
def LDB_postinc : LOAD<0x1,  "ldb", (outs GR64:$rD), (ins memsrc10_postinc:$src)>;
def LDW_postinc : LOAD<0x5,  "ldw", (outs GR64:$rD), (ins memsrc10_postinc:$src)>;
def LDB_predec  : LOAD<0x2,  "ldb", (outs GR64:$rD), (ins memsrc10_predec:$src)>;
def LDW_predec  : LOAD<0x6,  "ldw", (outs GR64:$rD), (ins memsrc10_predec:$src)>;

def LDB_short : LOAD_short<0x0, "ldb", (outs GR8:$rD), (ins memsrc3:$src)>;
def LDW_short : LOAD_short<0x4, "ldw", (outs GR8:$rD), (ins memsrc3:$src)>;
def LDB_postinc_short : LOAD_short<0x1, "ldb", (outs GR8:$rD), (ins memsrc3_postinc:$src)>;
def LDW_postinc_short : LOAD_short<0x5, "ldw", (outs GR8:$rD), (ins memsrc3_postinc:$src)>;
def LDB_predec_short  : LOAD_short<0x2, "ldb", (outs GR8:$rD), (ins memsrc3_predec:$src)>;
def LDW_predec_short  : LOAD_short<0x6, "ldw", (outs GR8:$rD), (ins memsrc3_predec:$src)>;
} // end of mayLoad

// Load patterns
def : Pat<(i16 (zextloadi8 GR64:$src)), (LDB GR64:$src, (i16 0))>;
def : Pat<(i16 (zextloadi8 addr_MO10:$src)), (LDB addr_MO10:$src)>;

def : Pat<(i16 (extloadi8 GR64:$src)), (LDB GR64:$src, (i16 0))>;
def : Pat<(i16 (extloadi8 addr_MO10:$src)), (LDB addr_MO10:$src)>;

def : Pat<(i16 (load GR64:$src)), (LDW GR64:$src, (i16 0))>;
def : Pat<(i16 (load addr_MO10:$src)), (LDW addr_MO10:$src)>;

// Short load patterns
// Select over long loads
let AddedComplexity = 1 in {
  def : Pat<(i16 (zextloadi8 GR8:$src)), (LDB_short GR8:$src, (i16 0))>;
  def : Pat<(i16 (zextloadi8 addr_MO3:$src)), (LDB_short addr_MO3:$src)>;

  def : Pat<(i16 (extloadi8 GR8:$src)), (LDB_short GR8:$src, (i16 0))>;
  def : Pat<(i16 (extloadi8 addr_MO3:$src)), (LDB_short addr_MO3:$src)>;

  def : Pat<(i16 (load GR8:$src)), (LDW_short GR8:$src, (i16 0))>;
  def : Pat<(i16 (load addr_MO3:$src)), (LDW_short addr_MO3:$src)>;
}

let mayStore = 1 in {
def STB : STORE<0x8,  "stb", (outs), (ins memsrc10:$dst, GR64:$rA)>;
def STW : STORE<0xc,  "stw", (outs), (ins memsrc10:$dst, GR64:$rA)>;
def STB_postinc : STORE<0x9,  "stb", (outs), (ins memsrc10_postinc:$dst, GR64:$rA)>;
def STW_postinc : STORE<0xd,  "stw", (outs), (ins memsrc10_postinc:$dst, GR64:$rA)>;
def STB_predec  : STORE<0xa,  "stb", (outs), (ins memsrc10_predec:$dst, GR64:$rA)>;
def STW_predec  : STORE<0xe,  "stw", (outs), (ins memsrc10_predec:$dst, GR64:$rA)>;

def STB_short : STORE_short<0x8, "stb", (outs), (ins memsrc3:$dst, GR8:$rA)>;
def STW_short : STORE_short<0xc, "stw", (outs), (ins memsrc3:$dst, GR8:$rA)>;
def STB_postinc_short : STORE_short<0x9, "stb", (outs), (ins memsrc3_postinc:$dst, GR8:$rA)>;
def STW_postinc_short : STORE_short<0xd, "stw", (outs), (ins memsrc3_postinc:$dst, GR8:$rA)>;
def STB_predec_short  : STORE_short<0xa, "stb", (outs), (ins memsrc3_predec:$dst, GR8:$rA)>;
def STW_predec_short  : STORE_short<0xe, "stw", (outs), (ins memsrc3_predec:$dst, GR8:$rA)>;
} // end of mayStore

// Store patterns
def : Pat<(truncstorei8 GR64:$src, GR64:$dst), (STB GR64:$dst, (i16 0), GR64:$src)>;
def : Pat<(truncstorei8 GR64:$src, addr_MO10:$dst), (STB addr_MO10:$dst, GR64:$src)>;

def : Pat<(store GR64:$src, GR64:$dst), (STW GR64:$dst, (i16 0), GR64:$src)>;
def : Pat<(store GR64:$src, addr_MO10:$dst), (STW addr_MO10:$dst, GR64:$src)>;

// Short store patterns
// Select over long stores
let AddedComplexity = 1 in {
  def : Pat<(truncstorei8 GR8:$src, GR8:$dst), (STB_short GR8:$dst, (i16 0), GR8:$src)>;
  def : Pat<(truncstorei8 GR8:$src, addr_MO3:$dst), (STB_short addr_MO3:$dst, GR8:$src)>;

  def : Pat<(store GR8:$src, GR8:$dst), (STW_short GR8:$dst, (i16 0), GR8:$src)>;
  def : Pat<(store GR8:$src, addr_MO3:$dst), (STW_short addr_MO3:$dst, GR8:$src)>;
}


//===----------------------------------------------------------------------===//
// Branch Operations
//===----------------------------------------------------------------------===//

multiclass BRCC<bits<4> cc, string opname> {
  def _ : Inst_i10_rr
    <0x2, cc, (outs), (ins brtarget:$imm, GR64:$rA, GR64:$rB),
      !strconcat(opname, "\t$imm, $rA, $rB"), []>;

  // Don't assemble short branch instructions
  let isCodeGenOnly = 1 in {
    def _short : Inst_i3_rr_short
      <0x2, cc, (outs), (ins brtarget:$imm, GR8:$rA, GR8:$rB),
        !strconcat(opname, "\t$imm, $rA, $rB"), []>;
  }
}

let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
  defm BEQ   : BRCC<0x2, "beq">;
  defm BNE   : BRCC<0x3, "bne">;
  defm BLTS  : BRCC<0x4, "blts">;
  defm BLES  : BRCC<0x5, "bles">;
  defm BLTU  : BRCC<0x6, "bltu">;
  defm BLEU  : BRCC<0x7, "bleu">;

  def BRA : Inst_i18
    <0x2, 0x0, (outs), (ins brtarget:$imm), "bra\t$imm", [(br bb:$imm)]>;

  // Don't assemble short branch instructions
  let isCodeGenOnly = 1 in {
    def BRA_short : Inst_i9_short
      <0x2, 0x0, (outs), (ins brtarget:$imm), "bra\t$imm", []>;
  }
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

// Mark R0 as a def as it is the link register
let isCall = 1, Uses = [R1], Defs = [R0] in {

let isCodeGenOnly = 1 in
  def BAL_short : Inst_i6_r_short
    <0x2, 0x1, (outs), (ins i16imm:$imm, GR8:$rB), "bal\t$imm, $rB", []>;

def BAL : Inst_i16_r
  <0x2, 0x1, (outs), (ins i16imm:$imm, GR64:$rB), "bal\t$imm, $rB", []>;

def JAL_short : Inst_rrr_short
  <0x2, 0x9, (outs), (ins GR8:$rD, GR64:$rB), "jal\t$rD, $rB", []> {
    let rA = 0;
  }

def JAL : Inst_rrr
  <0x2, 0x9, (outs), (ins GR64:$rD, GR64:$rB), "jal\t$rD, $rB", []> {
    let rA = 0;
  }
} // End of isCall, Uses, Defs

// Calls are encoded as a branch through the link register
def : Pat<(callflag (i16 imm:$imm), GR64:$rB), (BAL imm:$imm, GR64:$rB)>;


let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
def JMP : Inst_r<0x2, 0x8, (outs), (ins GR64:$rD), "jmp\t$rD", []>;

def JMP_short : Inst_r_short<0x2, 0x8, (outs), (ins GR8:$rD), "jmp\t$rD", []>;
}

// Indirect calls are a jump through a register
def : Pat<(callflag GR64:$rD, GR64:$rB), (JAL GR64:$rD, GR64:$rB)>;

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
def : Pat<(addc GR64:$src1, GR64:$src2), (ADD_r GR64:$src1, GR64:$src2)>;

def : Pat<(i16 (aapwrapper tglobaladdr:$dst)), (MOVI_i16 tglobaladdr:$dst)>;
def : Pat<(i16 (aapwrapper texternalsym:$dst)), (MOVI_i16 texternalsym:$dst)>;
def : Pat<(i16 (aapwrapper tblockaddress:$dst)), (MOVI_i16 tblockaddress:$dst)>;
