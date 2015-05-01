//===- AAPInstrFormats.td - AAP Instruction Formats ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Instruction Format Superclass
//===----------------------------------------------------------------------===//
class InstAAP<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
              string asmstr, list<dag> pattern>
  : Instruction {
  let Namespace = "AAP";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let Size = 2;
  field bits<16> Inst;
  field bits<16> SoftFail = 0;

  let Inst{15} = 0;
  let Inst{14-13} = opclass;
  let Inst{12-9}  = opcode;
}


class InstAAP_l<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                string asmstr, list<dag> pattern>
  : Instruction {
  let Namespace = "AAP";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let Size = 4;
  field bits<32> Inst;
  field bits<32> SoftFail = 0;

  let Inst{31} = 0;
  let Inst{15} = 1;
  let Inst{14-13} = opclass{1-0};
  let Inst{30-29} = opclass{3-2};
  let Inst{12-9}  = opcode{3-0};
  let Inst{28-25} = opcode{7-4};
}


//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//
// OP rD, rA, rB
class Inst_rrr<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
               string asmstr, list<dag> pattern>
  : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<3> rA;
  bits<3> rB;
  let Inst{8-6} = rD;
  let Inst{5-3} = rA;
  let Inst{2-0} = rB;
}

// OP rD, rA, imm
class Inst_rri<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
               string asmstr, list<dag> pattern>
  : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<3> rA;
  bits<3> imm;
  let Inst{8-6} = rD;
  let Inst{5-3} = rA;
  let Inst{2-0} = imm;
}

// OP rD, imm
class Inst_ri<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
              string asmstr, list<dag> pattern>
  : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<6> imm;
  let Inst{8-6} = rD;
  let Inst{5-0} = imm;
}

// OP imm, rA, rB
class Inst_irr<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
               string asmstr, list<dag> pattern>
  : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> imm;
  bits<3> rA;
  bits<3> rB;
  let Inst{8-6} = imm;
  let Inst{5-3} = rA;
  let Inst{2-0} = rB;
}

// OP imm
class Inst_i<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
             string asmstr, list<dag> pattern>
  : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<9> imm;
  let Inst{8-0} = imm;
}


// OP rD, rA, rB
class Inst_rrrl<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                string asmstr, list<dag> pattern>
  : InstAAP_l<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<6> rB;
  let Inst{8-6}   = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3}   = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0}   = rB{2-0};
  let Inst{18-16} = rB{5-3};
}

// OP rD, rA, imm
class Inst_rril<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                string asmstr, list<dag> pattern>
  : InstAAP_l<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<6> imm;
  let Inst{8-6}   = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3}   = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0}   = imm{2-0};
  let Inst{18-16} = imm{5-3};
}

// OP rD, imm
class Inst_ril<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
               string asmstr, list<dag> pattern>
  : InstAAP_l<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6>  rD;
  bits<12> imm;
  let Inst{8-6}   = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-0}   = imm{5-0};
  let Inst{21-16} = imm{11-6};
}

// OP imm, rA, rB
class Inst_irrl<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                string asmstr, list<dag> pattern>
  : InstAAP_l<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> imm;
  bits<6> rA;
  bits<6> rB;
  let Inst{8-6}   = imm{2-0};
  let Inst{24-22} = imm{5-3};
  let Inst{5-3}   = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0}   = rB{2-0};
  let Inst{18-16} = rB{5-3};
}

// OP imm
class Inst_il<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
              string asmstr, list<dag> pattern>
  : InstAAP_l<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<18> imm;
  let Inst{8-0}   = imm{8-0};
  let Inst{24-16} = imm{17-9};
}


let isCodeGenOnly = 1 in {
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : InstAAP_l<0x0, 0x0, outs, ins, asmstr, pattern> {
  let Inst{31-0} = 0;
}
}




