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
def sdt_call         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def callflag         : SDNode<"AAPISD::CALL", sdt_call,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def retflag          : SDNode<"AAPISD::RET_FLAG", SDTNone,
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


//===----------------------------------------------------------------------===//
// MOV Operations
//===----------------------------------------------------------------------===//
def MOV_rr : Inst_rrr<0x0, 0x9, (outs GR8:$rD), (ins GR8:$rA),
                      "mov\t$rD, $rA", []> {
  let Inst{2-0} = 0;  // No rB
}

def MOV_ri : Inst_ri<0x0, 0xf, (outs GR8:$rD), (ins i16imm:$imm),
                     "mov\t$rD, $imm",
                     [(set GR8:$rD, (i16 imm:$imm))]>;

def MOV_rrl : Inst_rrrl<0x0, 0x9, (outs GR64:$rD), (ins GR64:$rA),
                        "mov\t$rD, $rA",
                        []> {
  // No rB
  let Inst{2-0}   = 0;
  let Inst{18-16} = 0;
}

def MOV_ril : InstAAP_l<0x0, 0xf, (outs GR64:$rD), (ins i16imm:$imm),
                        "mov\t$rD, $imm",
                        [(set GR64:$rD, (i16 imm:$imm))]> {
  bits<6>  rD;
  bits<16> imm;
  let Inst{8-6}   = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-0}   = imm{5-0};
  let Inst{21-16} = imm{11-6};

  // Reuse top half of opcode for extended immediate
  let Inst{28-25} = imm{15-12};
}


//===----------------------------------------------------------------------===//
// ALU Operations
//===----------------------------------------------------------------------===//

class ALU_rrr<bits<4> opcode, string opname, SDNode OpNode>
  : Inst_rrr<0x0, opcode, (outs GR8:$rD), (ins GR8:$rA, GR8:$rB),
             !strconcat(opname, "\t$rD, $rA, $rB"),
             [(set GR8:$rD, (OpNode GR8:$rA, GR8:$rB))]>;

class ALU_rri<bits<4> opcode, string opname, SDNode OpNode>
  : Inst_rrr<0x0, opcode, (outs GR8:$rD), (ins GR8:$rA, i16imm:$imm),
             !strconcat(opname, "\t$rD, $rA, $imm"),
             [(set GR8:$rD, (OpNode GR8:$rA, imm:$imm))]>;

class ALU_rrrl<bits<8> opcode, string opname, SDNode OpNode>
  : Inst_rrrl<0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, GR64:$rB),
              !strconcat(opname, "\t$rD, $rA, $rB"),
              [(set GR64:$rD, (OpNode GR64:$rA, GR64:$rB))]>;

class ALU_rril<bits<8> opcode, string opname, SDNode OpNode>
  : Inst_rril<0x0, opcode, (outs GR64:$rD), (ins GR64:$rA, i16imm:$imm),
              !strconcat(opname, "\t$rD, $rA, $imm"),
              [(set GR64:$rD, (OpNode GR64:$rA, imm:$imm))]>;


multiclass ALU_ALL_rrr<bits<8> opcode, string opname, SDNode OpNode> {
  def _rrr  : ALU_rrr<opcode{3-0}, opname, OpNode>;
  def _rrrl : ALU_rrrl<opcode{7-0}, opname, OpNode>;
}

multiclass ALU_ALL_rri<bits<8> opcode, string opname, SDNode OpNode> {
  def _rri  : ALU_rri<opcode{3-0}, opname, OpNode>;
  def _rril : ALU_rril<opcode{7-0}, opname, OpNode>;
}

let isCommutable = 1 in {
  defm ADD : ALU_ALL_rrr<0x1, "add", add>;
  defm AND : ALU_ALL_rrr<0x3, "and", and>;
  defm OR  : ALU_ALL_rrr<0x4, "or",  or>;
  defm XOR : ALU_ALL_rrr<0x5, "or",  or>;

  defm ADD : ALU_ALL_rri<0xa, "add", add>;

  def ADDC_rrrl : ALU_rrrl<0x10, "addc", addc>;
}


defm SUB : ALU_ALL_rrr<0x2, "sub", sub>;
defm ASR : ALU_ALL_rrr<0x6, "asr", sra>;
defm LSL : ALU_ALL_rrr<0x7, "lsl", shl>;
defm LSR : ALU_ALL_rrr<0x8, "lsr", srl>;

defm SUB : ALU_ALL_rri<0xb, "sub", sub>;
defm ASR : ALU_ALL_rri<0xc, "asr", sra>;
defm LSL : ALU_ALL_rri<0xd, "lsl", shl>;
defm LSR : ALU_ALL_rri<0xe, "lsr", srl>;

def SUBC_rrrl : ALU_rrrl<0x11, "subc", subc>;


//===----------------------------------------------------------------------===//
// Load and Store Operations
//===----------------------------------------------------------------------===//


def addr_MO6 : ComplexPattern<iPTR, 2, "SelectAddr_MO6", [], []>;

def memsrcAsmOperand : AsmOperandClass { let Name = "MemSrc"; }

def memsrc : Operand<i16> {
  let PrintMethod   = "printMemSrcOperand";
  let EncoderMethod = "encodeMemSrcOperand";
  let ParserMatchClass = memsrcAsmOperand;
  //let DecoderMethod = "decodeSrcMemOperand";
  let MIOperandInfo = (ops GR8, i16imm);
}
def memdst : Operand<i16> {
  let PrintMethod   = "printMemSrcOperand";
  let EncoderMethod = "encodeMemSrcOperand";
  let ParserMatchClass = memsrcAsmOperand;
  //let DecoderMethod = "decodeDstMemOperand";
  let MIOperandInfo = (ops GR8, i16imm);
}


class LD_rm<bits<4> opcode, string asmstr>
  : InstAAP<0x1, opcode, (outs GR8:$rD), (ins memsrc:$src), asmstr, []> {
  bits<3> rD;
  bits<6> src;
  let Inst{8-6} = rD;
  let Inst{5-3} = src{2-0}; // rA
  let Inst{2-0} = src{5-3}; // Imm
}


class LD_rml<bits<8> opcode, string asmstr, list<dag> pat>
  : InstAAP_l<0x1, opcode, (outs GR64:$rD), (ins memsrc:$src), asmstr, pat> {
  bits<6>  rD;
  bits<12> src;
  let Inst{8-6}   = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3}   = src{2-0};  // rA
  let Inst{21-19} = src{5-3};
  let Inst{2-0}   = src{8-6};  // imm
  let Inst{18-16} = src{11-9};
}


class ST_mr<bits<4> opcode, string asmstr>
  : InstAAP<0x1, opcode, (outs), (ins memdst:$dst, GR8:$rA), asmstr, []> {
  bits<6> dst;
  bits<3> rA;
  let Inst{8-6} = dst{2-0}; // rD
  let Inst{2-0} = dst{5-3}; // imm
  let Inst{5-3} = rA;
}


class ST_mrl<bits<8> opcode, string asmstr, list<dag> pat>
    : InstAAP_l<0x1, opcode, (outs), (ins memdst:$dst, GR64:$rA), asmstr, pat> {
  bits<12> dst;
  bits<6>  rA;
  let Inst{8-6}   = dst{2-0};  // rD
  let Inst{24-22} = dst{5-3};
  let Inst{2-0}   = dst{8-6};  // imm
  let Inst{18-16} = dst{11-9};
  let Inst{5-3}   = rA{2-0};
  let Inst{21-19} = rA{5-3};
}


// Simple loads
let mayLoad = 1 in {
def LDB : LD_rm<0x0, "ldb\t$rD, (${src})">;
def LDW : LD_rm<0x4, "ldw\t$rD, (${src})">;

def LDB_l : LD_rml<0x00, "ldb\t$rD, (${src})", []>;
def LDW_l : LD_rml<0x04, "ldw\t$rD, (${src})", [(set GR64:$rD, (load addr_MO6:$src))]>;
def LDD_l : LD_rml<0x40, "ldd\t$rD, (${src})", []>;
}


// Complex loads
let mayLoad = 1, Constraints = "$rA = $rA_wb" in {
def LDB_postinc : Inst_rri<0x1, 0x1, (outs GR8:$rD, GR8:$rA_wb), (ins GR8:$rA, i16imm:$imm),
                           "ldb\t$rD, (${rA}+, ${imm})", []>;
def LDW_postinc : Inst_rri<0x1, 0x5, (outs GR8:$rD, GR8:$rA_wb), (ins GR8:$rA, i16imm:$imm),
                           "ldw\t$rD, (${rA}+, ${imm})", []>;

def LDB_postinc_l : Inst_rril<0x1, 0x01, (outs GR64:$rD, GR64:$rA_wb), (ins GR64:$rA, i16imm:$imm),
                              "ldb\t$rD, (${rA}+, ${imm})", []>;
def LDW_postinc_l : Inst_rril<0x1, 0x05, (outs GR64:$rD, GR64:$rA_wb), (ins GR64:$rA, i16imm:$imm),
                              "ldw\t$rD, (${rA}+, ${imm})", []>;
def LDD_postinc_l : Inst_rril<0x1, 0x41, (outs GR64:$rD, GR64:$rA_wb), (ins GR64:$rA, i16imm:$imm),
                              "ldd\t$rD, (${rA}+, ${imm})", []>;


def LDB_predec : Inst_rri<0x1, 0x2, (outs GR8:$rD, GR8:$rA_wb), (ins GR8:$rA, i16imm:$imm),
                          "ldb\t$rD, (-${rA}, ${imm})", []>;
def LDW_predec : Inst_rri<0x1, 0x7, (outs GR8:$rD, GR8:$rA_wb), (ins GR8:$rA, i16imm:$imm),
                          "ldw\t$rD, (-${rA}, ${imm})", []>;

def LDB_predec_l : Inst_rril<0x1, 0x2, (outs GR64:$rD, GR64:$rA_wb), (ins GR64:$rA, i16imm:$imm),
                             "ldb\t$rD, (-${rA}, ${imm})", []>;
def LDW_predec_l : Inst_rril<0x1, 0x7, (outs GR64:$rD, GR64:$rA_wb), (ins GR64:$rA, i16imm:$imm),
                             "ldw\t$rD, (-${rA}, ${imm})", []>;
}


// Simple stores
let mayStore = 1 in {
def STB : ST_mr<0x8, "stb\t(${dst}), $rA">;
def STW : ST_mr<0xc, "stw\t(${dst}), $rA">;

def STB_l : ST_mrl<0x08, "stb\t(${dst}), $rA", []>;
def STW_l : ST_mrl<0x0c, "stw\t(${dst}), $rA", [(store GR64:$rA, addr_MO6:$dst)]>;
def STD_l : ST_mrl<0x48, "std\t(${dst}), $rA", []>;
}


// Complex stores
let mayStore = 1, Constraints = "$rD = $rD_wb" in {
def STB_postinc : Inst_rri<0x1, 0x9, (outs GR8:$rD_wb), (ins GR8:$rD, i16imm:$imm, GR8:$rA),
                           "stb\t(${rD}+, ${imm}), $rA", []>;
def STW_postinc : Inst_rri<0x1, 0xd, (outs GR8:$rD_wb), (ins GR8:$rD, i16imm:$imm, GR8:$rA),
                           "stw\t(${rD}+, ${imm}), $rA", []>;

def STB_postinc_l : Inst_rril<0x1, 0x09, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "stw\t(${rD}+, ${imm}), $rA", []>;
def STW_postinc_l : Inst_rril<0x1, 0x0d, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "stw\t(${rD}+, ${imm}), $rA", []>;
def STD_postinc_l : Inst_rril<0x1, 0x49, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "std\t(${rD}+, ${imm}), $rA", []>;


def STB_predec : Inst_rri<0x1, 0xa, (outs GR8:$rD_wb), (ins GR8:$rD, i16imm:$imm, GR8:$rA),
                           "stb\t(-${rD}, ${imm}), $rA", []>;
def STW_predec : Inst_rri<0x1, 0xf, (outs GR8:$rD_wb), (ins GR8:$rD, i16imm:$imm, GR8:$rA),
                           "stw\t(-${rD}, ${imm}), $rA", []>;

def STB_predec_l : Inst_rril<0x1, 0x0a, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "stw\t(-${rD}, ${imm}), $rA", []>;
def STW_predec_l : Inst_rril<0x1, 0x0f, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "stw\t(-${rD}, ${imm}), $rA", []>;
def STD_predec_l : Inst_rril<0x1, 0x4a, (outs GR64:$rD_wb), (ins GR64:$rD, i16imm:$imm, GR64:$rA),
                              "std\t(-${rD}, ${imm}), $rA", []>;
}


// Simple load store patterns
def : Pat<(load (i16 GR64:$rA)), (LDW_l GR64:$rA, (i16 0))>;

def : Pat<(store (i16 GR64:$rA), (i16 GR64:$rD)),  (STW_l GR64:$rD, (i16 0), GR64:$rA)>;


//===----------------------------------------------------------------------===//
// Branch Operations
//===----------------------------------------------------------------------===//
multiclass BRCC<bits<8> cc, string opname> {
  def _s : Inst_irr<0x2, cc{3-0}, (outs), (ins brtarget:$imm, GR8:$rA, GR8:$rB),
                    !strconcat(opname, "\t$imm, $rA, $rB"), []>;
  def _l : Inst_irrl<0x2, cc, (outs), (ins brtarget:$imm, GR64:$rA, GR64:$rB),
                     !strconcat(opname, "\t$imm, $rA, $rB"), []>;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
defm BEQ   : BRCC<0x2, "beq">;
defm BNE   : BRCC<0x3, "bne">;
defm BLTS  : BRCC<0x4, "blts">;
defm BGTS  : BRCC<0x5, "bgts">;
defm BLTU  : BRCC<0x6, "bltu">;
defm BGTU  : BRCC<0x7, "bgtu">;

def BRA   : Inst_i<0x2, 0x0, (outs), (ins brtarget:$imm),
                   "bra\t$imm", []>;
def BRA_l : Inst_il<0x2, 0x0, (outs), (ins brtarget:$imm),
                    "bra\t$imm",
                    [(br bb:$imm)]>;
}

// Implements the BR_CC DAG node. Expanded into one of the above branches.
let isBranch = 1, isTerminator = 1, isBarrier = 1, usesCustomInserter = 1 in {
def BR_CC   : Pseudo<(outs), (ins i16imm:$cc, GR8:$lhs, GR8:$rhs, brtarget:$target),
                     "#BR_CC", []>;
def BR_CC_l : Pseudo<(outs), (ins i16imm:$cc, GR64:$lhs, GR64:$rhs, brtarget:$target),
                     "#BR_CC_l",
                     [(AAPbrcc imm:$cc, GR64:$lhs, GR64:$rhs, bb:$target)]>;
}

// Implements the SELECT_CC DAG node. Expanded into a branch sequence
let usesCustomInserter = 1 in {
def SELECT_CC : Pseudo<(outs GR64:$dst), (ins GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, i16imm:$cc),
                       "#SELECT_CC",
                       [(set GR64:$dst, (AAPselectcc GR64:$lhs, GR64:$rhs, GR64:$T, GR64:$F, imm:$cc))]>;
}


//===----------------------------------------------------------------------===//
// Calls and returns
//===----------------------------------------------------------------------===//
//FIXME: Mark as clobbering all non-callee saved regs
let isCall = 1,
    Uses = [R1],
    Defs = [R0,  R2,  R3,  R4,  R5,  R6,  R7,  R8,  R9, R10, R11, R12,
            R13, R14, R15, R16, R17, R18, R19] in {
def JAL : Inst_rrr<0x2, 0x8, (outs), (ins i16imm:$rD),
                   "jal\t$rD", [(callflag imm:$rD)]> {
  // No rA or rB
  let Inst{5-0} = 0;
}
def JAL_l : Inst_rrrl<0x2, 0x8, (outs), (ins i16imm:$rD),
                      "jal\t$rD", [(callflag imm:$rD)]> {
  // No rA or rB
  let Inst{5-0}   = 0;
  let Inst{21-16} = 0;
}
}

// Returns are encoded as a jump through the link register (r0)
let Uses = [R0], isReturn = 1, isTerminator = 1, isBarrier = 1 in {
def RET : InstAAP<0x2, 0x9, (outs), (ins), "jmp\tr0", [(retflag)]> {
  let Inst{8-6} = 0;  // rD = link register (r0)
  let Inst{5-0} = 0;  // zero other operands
}
}

// Stack adjustment
let Defs = [R1], Uses = [R1] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i16imm:$amt),
                              "#ADJCALLSTACKDOWN",
                              [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i16imm:$amt1, i16imm:$amt2),
                              "#ADJCALLSTACKUP",
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
}


//===----------------------------------------------------------------------===//
// NOP Operation
//===----------------------------------------------------------------------===//
def NOP : Inst_ri<0x0, 0x0, (outs), (ins GR8:$rD, i16imm:$imm),
          "nop\t$rD, $imm", []>;

def NOP_l : Inst_ril<0x0, 0x0, (outs), (ins GR64:$rD, i16imm:$imm),
          "nop\t$rD, $imm", []>;


//===----------------------------------------------------------------------===//
// Peephole Patterns
//===----------------------------------------------------------------------===//
def : Pat<(i16 (aapwrapper tglobaladdr:$dst)), (MOV_ril tglobaladdr:$dst)>;
def : Pat<(i16 (aapwrapper texternalsym:$dst)), (MOV_ril texternalsym:$dst)>;
def : Pat<(i16 (aapwrapper tblockaddress:$dst)), (MOV_ril tblockaddress:$dst)>;

// Calls
def : Pat<(callflag (i16 tglobaladdr:$dst)),  (JAL_l tglobaladdr:$dst)>;
def : Pat<(callflag (i16 texternalsym:$dst)), (JAL_l texternalsym:$dst)>;
