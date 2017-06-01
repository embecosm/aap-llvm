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
def sdt_call : SDTypeProfile<0, -1, [SDTCisVT<1, iPTR>, SDTCisVT<1, i16>]>;
def sdt_ret  : SDTypeProfile<0,  1, [SDTCisVT<0, i16>]>;
def callflag : SDNode<"AAPISD::CALL", sdt_call,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def retflag : SDNode<"AAPISD::RET_FLAG", sdt_ret,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def sdt_callseqstart : SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def sdt_callseqend   : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", sdt_callseqstart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", sdt_callseqend,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def sdt_wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def aapwrapper : SDNode<"AAPISD::Wrapper", sdt_wrapper>;

def sdt_selectcc : SDTypeProfile<1, 5, [SDTCisSameAs<0, 3>,
                                        SDTCisSameAs<1, 2>,
                                        SDTCisSameAs<3, 4>,
                                        SDTCisVT<5, i16>]>;
def sdt_brcc     : SDTypeProfile<0, 4, [SDTCisVT<0, i16>,
                                        SDTCisSameAs<1, 2>,
                                        SDTCisVT<3, OtherVT>]>;
def AAPselectcc : SDNode<"AAPISD::SELECT_CC", sdt_selectcc>;
def AAPbrcc : SDNode<"AAPISD::BR_CC", sdt_brcc, [SDNPHasChain]>;

// Branch Operands
def brtarget : Operand<OtherVT> {
  let PrintMethod = "printPCRelImmOperand";
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
  def field16AsmOperand : AsmOperandClass {
    let Name = "Field16";
    let SuperClasses = [ImmAsmOperand];
  }
  def imm12AsmOperand : AsmOperandClass {
    let Name = "Imm12";
    let SuperClasses = [field16AsmOperand];
  }
  def imm10AsmOperand : AsmOperandClass {
    let Name = "Imm10";
    let SuperClasses = [imm12AsmOperand];
  }
  def imm9AsmOperand : AsmOperandClass {
    let Name = "Imm9";
    let SuperClasses = [imm10AsmOperand];
  }
  def const6AsmOperand : AsmOperandClass {
    let Name = "Const6";
    let SuperClasses = [imm9AsmOperand];
  }
  def imm6AsmOperand : AsmOperandClass {
    let Name = "Imm6";
    let SuperClasses = [const6AsmOperand];
  }
  def const3AsmOperand : AsmOperandClass {
    let Name = "Const3";
    let SuperClasses = [imm6AsmOperand];
  }

  // Offsets are signed immediates used for load and store operands
  def off10AsmOperand : AsmOperandClass {
    let Name = "Off10";
    let SuperClasses = [ImmAsmOperand];
  }
  def off3AsmOperand : AsmOperandClass {
    let Name = "Off3";
    let SuperClasses = [off10AsmOperand];
  }

  // Shift operands are unsigned immediates or constants who range
  // begins at 1, rather than 0. The encoding is done with the shift amount
  // -1.
  def shiftImm6AsmOperand : AsmOperandClass {
    let Name = "ShiftImm6";
    let SuperClasses = [imm9AsmOperand];
  }
  def shiftConst3AsmOperand : AsmOperandClass {
    let Name = "ShiftConst3";
    let SuperClasses = [shiftImm6AsmOperand];
  }
}

// The various AAP functions called in ImmLeaf here are defined in AAP.h
def field16 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isField16(Imm); }]> {
  let EncoderMethod = "encodeField16";
  let ParserMatchClass = field16AsmOperand;
}
def imm12 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm12(Imm); }]> {
  let EncoderMethod = "encodeImm12";
  let ParserMatchClass = imm12AsmOperand;
}
def imm10 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm10(Imm); }]> {
  let EncoderMethod = "encodeImm10";
  let ParserMatchClass = imm10AsmOperand;
}
def imm9 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm9(Imm); }]> {
  let EncoderMethod = "encodeImm9";
  let ParserMatchClass = imm9AsmOperand;
}
def const6 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm6(Imm); }]> {
  let EncoderMethod = "encodeImm6";
  let ParserMatchClass = const6AsmOperand;
}
def imm6 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm6(Imm); }]> {
  let EncoderMethod = "encodeImm6";
  let ParserMatchClass = imm6AsmOperand;
}
def const3 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isImm3(Imm); }]> {
  let EncoderMethod = "encodeImm3";
  let ParserMatchClass = const3AsmOperand;
}

// Offset operands
def off10 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isOff10(Imm); }]> {
  let EncoderMethod = "encodeOff10";
  let DecoderMethod = "decodeOff10";
  let ParserMatchClass = off10AsmOperand;
}
def off3 : Operand<i16>, ImmLeaf<i16, [{ return AAP::isOff3(Imm); }]> {
  let EncoderMethod = "encodeOff3";
  let DecoderMethod = "decodeOff3";
  let ParserMatchClass = off3AsmOperand;
}

// Shift operands, these have custom encoding/decoding to handle the bias
// applied to the value.
def shift_imm6 : Operand<i16>,
                 ImmLeaf<i16, [{ return AAP::isShiftImm6(Imm); }]> {
  let EncoderMethod = "encodeShiftImm6";
  let DecoderMethod = "decodeShiftOperand";
  let ParserMatchClass = shiftImm6AsmOperand;
}
def shift_const3 : Operand<i16>,
                   ImmLeaf<i16, [{ return AAP::isShiftImm3(Imm); }]> {
  let EncoderMethod = "encodeShiftConst3";
  let DecoderMethod = "decodeShiftOperand";
  let ParserMatchClass = shiftConst3AsmOperand;
}

// Memory offsets consist of a 3 or 10 bit offset and a register operand
def addr_MO3 : ComplexPattern<iPTR, 2, "SelectAddr_MO3", [], []>;
def addr_MO10 : ComplexPattern<iPTR, 2, "SelectAddr_MO10", [], []>;

// Memsrc operand encodings consist of a 16-bit immediate field in the low
// bits, and a 3 or 6 bit register field in the high bits. The encoding
// is exactly the same for predecrement and postincrement address mode.
//
// We enforce a total order between all of the memsrc operand classes using
// SuperClasses so that the shortest instruction is always picked first during
// assembly matching. Load and store instructions have identical mnemonics,
// and therefore their ordering is based on their operand classes.
def memsrc10AsmOperand : AsmOperandClass {
  let Name = "MemSrc10";
}
def memsrc10PostIncAsmOperand : AsmOperandClass {
  let Name = "MemSrc10PostInc";
  let SuperClasses = [memsrc10AsmOperand];
}
def memsrc10PreDecAsmOperand : AsmOperandClass {
  let Name = "MemSrc10PreDec";
  let SuperClasses = [memsrc10PostIncAsmOperand];
}

def memsrc3AsmOperand : AsmOperandClass {
  let Name = "MemSrc3";
  let SuperClasses = [memsrc10PreDecAsmOperand];
}
def memsrc3PostIncAsmOperand : AsmOperandClass {
  let Name = "MemSrc3PostInc";
  let SuperClasses = [memsrc3AsmOperand];
}
def memsrc3PreDecAsmOperand : AsmOperandClass {
  let Name = "MemSrc3PreDec";
  let SuperClasses = [memsrc3PostIncAsmOperand];
}

let EncoderMethod = "encodeMemSrcOperand",
    DecoderMethod = "decodeMemSrc10Operand" in {
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
}
let EncoderMethod = "encodeMemSrcOperand",
    DecoderMethod = "decodeMemSrc3Operand" in {
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

// May be exapanded to MOV_r_short in peephole pass
def MOV_r : Inst_rrr
  <0x0, 0x9, (outs GR64:$rD), (ins GR64:$rA), "mov\t$rD, $rA", []> {
  let rB = 0;
}
def MOV_r_short : Inst_rrr_short
  <0x0, 0x9, (outs GR8:$rD), (ins GR8:$rA), "mov\t$rD, $rA", []> {
  let rB = 0;
}

// May expand to MOVI_i6_short in peephole pass
def MOVI_i16 : Inst_r_i16
  <0x0, 0xf, (outs GR64:$rD), (ins field16:$imm), "movi\t$rD, $imm",
    [(set GR64:$rD, (i16 field16:$imm))]>;
def MOVI_i6_short : Inst_r_i6_short
  <0x0, 0xf, (outs GR8:$rD), (ins const6:$imm), "movi\t$rD, $imm", []>;


//===----------------------------------------------------------------------===//
// ALU/Logical Operations
//===----------------------------------------------------------------------===//

// May expand to NOP_short in peephole
def NOP : Inst_r_i12
  <0x0, 0x0, (outs), (ins GR64:$rD, imm12:$imm), "nop\t$rD, $imm", []>;
def NOP_short : Inst_r_i6_short
  <0x0, 0x0, (outs), (ins GR8:$rD, const6:$imm), "nop\t$rD, $imm", []>;


// ALU ops with register operands
multiclass ALU_r<bits<8> opcode, string opname, SDNode OpNode> {
  // May expand to ALU_r_short in peephole
  def _r : Inst_rrr
    <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
      !strconcat(opname, "\t$rD, $rA, $rB"),
      [(set GR64:$rD, (OpNode GR64:$rA, GR64:$rB))]>;
  def _r_short : Inst_rrr_short
    <0x0, opcode{3-0}, (outs GR8:$rD), (ins GR8:$rA, GR8:$rB),
      !strconcat(opname, "\t$rD, $rA, $rB"), []>;
}

let isCommutable = 1 in {
  let Defs = [PSW] in {
    defm ADD : ALU_r<0x1, "add", add>;
  }
  defm AND : ALU_r<0x3, "and", and>;
  defm OR  : ALU_r<0x4, "or",  or>;
  defm XOR : ALU_r<0x5, "xor", xor>;
}

let Defs = [PSW] in {
  defm SUB : ALU_r<0x2, "sub", sub>;
}
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
  def LEA : Pseudo<(outs GR64:$rD), (ins GR64:$rA, i16imm:$imm), "#LEA", []>;

  // Select over MOVI_i16 + ADD_r
  // May expand to ADDI_i3_short in peephole
  let AddedComplexity = 1 in {
    def ADDI_i10 : Inst_rr_i10
      <0x0, 0xa, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
        "addi\t$rD, $rA, $imm",
        [(set GR64:$rD, (add GR64:$rA, (i16 imm10:$imm)))]>;
  }
  def ADDI_i3_short : Inst_rr_i3_short
    <0x0, 0xa, (outs GR8:$rD), (ins GR8:$rA, const3:$imm),
      "addi\t$rD, $rA, $imm", []>;

  // Select over MOVI_i16 + SUB_r
  // May expand to SUBI_i3_short in peephole
  let AddedComplexity = 1 in {
    def SUBI_i10 : Inst_rr_i10
      <0x0, 0xb, (outs GR64:$rD), (ins GR64:$rA, imm10:$imm),
        "subi\t$rD, $rA, $imm",
        [(set GR64:$rD, (sub GR64:$rA, (i16 imm10:$imm)))]>;
  }
  def SUBI_i3_short : Inst_rr_i3_short
    <0x0, 0xb, (outs GR8:$rD), (ins GR8:$rA, const3:$imm),
      "subi\t$rD, $rA, $imm", []>;
}

multiclass SHIFT_i<bits<8> opcode, string opname, SDNode OpNode> {
  // Select over MOVI_i16 + SHIFT_r
  // May expand to SHIFT_i3_short in peephole
  let AddedComplexity = 1 in {
    def _i6 : Inst_rr_i6
      <0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, shift_imm6:$imm),
        !strconcat(opname, "\t$rD, $rA, $imm"),
        [(set GR64:$rD, (OpNode GR64:$rA, (i16 shift_imm6:$imm)))]>;
  }
  def _i3_short : Inst_rr_i3_short
    <0x0, opcode{3-0}, (outs GR8:$rD), (ins GR8:$rA, shift_const3:$imm),
      !strconcat(opname, "\t$rD, $rA, $imm"), []>;
}
defm ASRI : SHIFT_i<0xc, "asri", sra>;
defm LSLI : SHIFT_i<0xd, "lsli", shl>;
defm LSRI : SHIFT_i<0xe, "lsri", srl>;

// Logical operations with immediate
// Select over MOVI_i16 + LOG_r
let AddedComplexity = 1 in {
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
    : InstAAP<0x1, opcode, outs, ins, !strconcat(opname, "\t$rD, [$src]"), []> {
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
    : InstAAP_short<0x1, opcode, outs, ins, !strconcat(opname, "\t$rD, [$src]"),
                    []> {
  bits<3> rD;
  bits<19> src; // 3-bit reg field, 16 bit imm field

  let Inst{8-6} = rD;
  let Inst{5-3} = src{18-16};
  let Inst{2-0} = src{2-0};
}

class STORE<bits<8> opcode, string opname, dag outs, dag ins>
    : InstAAP<0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"), []> {
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
    : InstAAP_short<0x1, opcode, outs, ins, !strconcat(opname, "\t[$dst], $rA"),
                    []> {
  bits<19> dst; // 3-bit reg field, 16 bit imm field
  bits<3> rA;

  let Inst{8-6} = dst{18-16};
  let Inst{2-0} = dst{2-0};
  let Inst{5-3} = rA;
}

// Short load/store instructions use the memsrc3 operand type, which uses a
// different fixup when encoding and a different matching class when parsing
// assembly.

// Load instruction may expand to short equivalents in peephole
let mayLoad = 1 in {
  def LDB : LOAD
    <0x0,  "ldb", (outs GR64:$rD), (ins memsrc10:$src)>;
  def LDW : LOAD
    <0x4,  "ldw", (outs GR64:$rD), (ins memsrc10:$src)>;
  def LDB_postinc : LOAD
    <0x1,  "ldb", (outs GR64:$rD), (ins memsrc10_postinc:$src)>;
  def LDW_postinc : LOAD
    <0x5,  "ldw", (outs GR64:$rD), (ins memsrc10_postinc:$src)>;
  def LDB_predec : LOAD
    <0x2,  "ldb", (outs GR64:$rD), (ins memsrc10_predec:$src)>;
  def LDW_predec : LOAD
    <0x6,  "ldw", (outs GR64:$rD), (ins memsrc10_predec:$src)>;
}

let mayLoad = 1 in {
  def LDB_short : LOAD_short
    <0x0, "ldb", (outs GR8:$rD), (ins memsrc3:$src)>;
  def LDW_short : LOAD_short
    <0x4, "ldw", (outs GR8:$rD), (ins memsrc3:$src)>;
  def LDB_postinc_short : LOAD_short
    <0x1, "ldb", (outs GR8:$rD), (ins memsrc3_postinc:$src)>;
  def LDW_postinc_short : LOAD_short
    <0x5, "ldw", (outs GR8:$rD), (ins memsrc3_postinc:$src)>;
  def LDB_predec_short  : LOAD_short
    <0x2, "ldb", (outs GR8:$rD), (ins memsrc3_predec:$src)>;
  def LDW_predec_short  : LOAD_short
    <0x6, "ldw", (outs GR8:$rD), (ins memsrc3_predec:$src)>;
}

// Load patterns
def : Pat<(i16(zextloadi8 GR64:$src)), (LDB GR64:$src, (i16 0))>;
def : Pat<(i16(zextloadi8 addr_MO10:$src)), (LDB addr_MO10:$src)>;

def : Pat<(i16(extloadi8 GR64:$src)), (LDB GR64:$src, (i16 0))>;
def : Pat<(i16(extloadi8 addr_MO10:$src)), (LDB addr_MO10:$src)>;

def : Pat<(i16(load GR64:$src)), (LDW GR64:$src, (i16 0))>;
def : Pat<(i16(load addr_MO10:$src)), (LDW addr_MO10:$src)>;

// Store instruction may expand to short equivalents in peephole
let mayStore = 1 in {
  def STB : STORE
    <0x8,  "stb", (outs), (ins memsrc10:$dst, GR64:$rA)>;
  def STW : STORE
    <0xc,  "stw", (outs), (ins memsrc10:$dst, GR64:$rA)>;
  def STB_postinc : STORE
    <0x9,  "stb", (outs), (ins memsrc10_postinc:$dst, GR64:$rA)>;
  def STW_postinc : STORE
    <0xd,  "stw", (outs), (ins memsrc10_postinc:$dst, GR64:$rA)>;
  def STB_predec : STORE
    <0xa,  "stb", (outs), (ins memsrc10_predec:$dst, GR64:$rA)>;
  def STW_predec : STORE
    <0xe,  "stw", (outs), (ins memsrc10_predec:$dst, GR64:$rA)>;
}

let mayStore = 1 in {
  def STB_short : STORE_short
    <0x8, "stb", (outs), (ins memsrc3:$dst, GR8:$rA)>;
  def STW_short : STORE_short
    <0xc, "stw", (outs), (ins memsrc3:$dst, GR8:$rA)>;
  def STB_postinc_short : STORE_short
    <0x9, "stb", (outs), (ins memsrc3_postinc:$dst, GR8:$rA)>;
  def STW_postinc_short : STORE_short
    <0xd, "stw", (outs), (ins memsrc3_postinc:$dst, GR8:$rA)>;
  def STB_predec_short  : STORE_short
    <0xa, "stb", (outs), (ins memsrc3_predec:$dst, GR8:$rA)>;
  def STW_predec_short  : STORE_short
    <0xe, "stw", (outs), (ins memsrc3_predec:$dst, GR8:$rA)>;
} // end of mayStore

// Store patterns
def : Pat<(truncstorei8 GR64:$src, GR64:$dst), (STB GR64:$dst, (i16 0), GR64:$src)>;
def : Pat<(truncstorei8 GR64:$src, addr_MO10:$dst), (STB addr_MO10:$dst, GR64:$src)>;

def : Pat<(store GR64:$src, GR64:$dst), (STW GR64:$dst, (i16 0), GR64:$src)>;
def : Pat<(store GR64:$src, addr_MO10:$dst), (STW addr_MO10:$dst, GR64:$src)>;


//===----------------------------------------------------------------------===//
// Branch Operations
//===----------------------------------------------------------------------===//

multiclass BRCC<bits<4> cc, string opname> {
  // Branches may expand to short equivalents in peephole
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
  defm BEQ  : BRCC<0x2, "beq">;
  defm BNE  : BRCC<0x3, "bne">;
  defm BLTS : BRCC<0x4, "blts">;
  defm BLES : BRCC<0x5, "bles">;
  defm BLTU : BRCC<0x6, "bltu">;
  defm BLEU : BRCC<0x7, "bleu">;

  // BRA may expand to short equivalent in the peephole
  def BRA : Inst_i22
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
}

// Implements the SELECT_CC DAG node. Expanded into a branch sequence
let usesCustomInserter = 1 in {
  def SELECT_CC : Pseudo
    <(outs GR64:$dst), (ins GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, i16imm:$cc),
      "#SELECT_CC",
      [(set GR64:$dst,
        (AAPselectcc GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, imm:$cc))]>;
}

//===----------------------------------------------------------------------===//
// Calls and returns
//===----------------------------------------------------------------------===//

// Mark R0 as a def as it is the link register
let isCall = 1, Uses = [R1], Defs = [R0] in {
  // Never assemble short branches
  let isCodeGenOnly = 1 in {
    def BAL_short : Inst_i6_r_short
      <0x2, 0x1, (outs), (ins i16imm:$imm, GR8:$rB), "bal\t$imm, $rB", []>;
  }

  // BAL may expand to BAL_short in peephole
  def BAL : Inst_i16_r
    <0x2, 0x1, (outs), (ins i16imm:$imm, GR64:$rB), "bal\t$imm, $rB", []>;

  def JAL_short : Inst_rrr_short
    <0x2, 0x9, (outs), (ins GR8:$rD, GR64:$rB), "jal\t$rD, $rB", []> {
      let rA = 0;
  }

  // JAL may expand to JAL_short in peephole
  def JAL : Inst_rrr
    <0x2, 0x9, (outs), (ins GR64:$rD, GR64:$rB), "jal\t$rD, $rB", []> {
      let rA = 0;
  }
}

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  // May expand to JMP_short in peephole
  def JMP :
    Inst_r<0x2, 0x8, (outs), (ins GR64:$rD), "jmp\t$rD", []>;
  def JMP_short :
    Inst_r_short<0x2, 0x8, (outs), (ins GR8:$rD), "jmp\t$rD", []>;
}

// Indirect calls are a jump through a register
def : Pat<(callflag GR64:$rD, GR64:$rB), (JAL GR64:$rD, GR64:$rB)>;

// Returns are encoded as a jump through the link register
def : Pat<(retflag GR64:$rD), (JMP GR64:$rD)>;

// Stack adjustment
let Defs = [R1], Uses = [R1] in {
  def ADJCALLSTACKDOWN : Pseudo
    <(outs), (ins i16imm:$amt, i16imm:$amt2), "#ADJCALLSTACKDOWN",
      [(callseq_start timm:$amt, timm:$amt2)]>;

  def ADJCALLSTACKUP : Pseudo
    <(outs), (ins i16imm:$amt1, i16imm:$amt2), "#ADJCALLSTACKUP",
      [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// Call patterns
def : Pat<(callflag (i16 tglobaladdr:$dst), GR64:$rA),
            (BAL tglobaladdr:$dst, GR64:$rA)>;
def : Pat<(callflag (i16 texternalsym:$dst), GR64:$rA),
            (BAL texternalsym:$dst, GR64:$rA)>;


//===----------------------------------------------------------------------===//
// Peephole Patterns
//===----------------------------------------------------------------------===//
def : Pat<(addc GR64:$src1, GR64:$src2), (ADD_r GR64:$src1, GR64:$src2)>;

def : Pat<(i16 (aapwrapper tglobaladdr:$dst)), (MOVI_i16 tglobaladdr:$dst)>;
def : Pat<(i16 (aapwrapper texternalsym:$dst)), (MOVI_i16 texternalsym:$dst)>;
def : Pat<(i16 (aapwrapper tblockaddress:$dst)), (MOVI_i16 tblockaddress:$dst)>;
