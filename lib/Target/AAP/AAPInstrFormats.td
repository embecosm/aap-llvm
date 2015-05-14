//===- AAPInstrFormats.td - AAP Instruction Formats ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Instruction Format Superclasses
//===----------------------------------------------------------------------===//

class InstAAP_base<bits<4> opclass, dag outs, dag ins,
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
}


class InstAAP<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
              string asmstr, list<dag> pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  let Inst{12-9}  = opcode{3-0};
  let Inst{28-25} = opcode{7-4};
}


class InstAAP_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
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


//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

class Inst_rrr<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
               string asmstr, list<dag> pattern>
    : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<6> rB;
  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0} = rB{2-0};
  let Inst{18-16} = rB{5-3};
}
class Inst_rrr_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                     string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<3> rA;
  bits<3> rB;
  let Inst{8-6} = rD;
  let Inst{5-3} = rA;
  let Inst{2-0} = rB;
}


class Inst_rr<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
             string asmstr, list<dag> pattern>
    : Inst_rrr<opclass, opcode, outs, ins, asmstr, pattern> {
  let rB = 0;
}
class Inst_rr_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                   string asmstr, list<dag> pattern>
    : Inst_rrr_short<opclass, opcode, outs, ins, asmstr, pattern> {
  let rB = 0;
}


class Inst_r<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
             string asmstr, list<dag> pattern>
    : Inst_rr<opclass, opcode, outs, ins, asmstr, pattern> {
  let rA = 0;
}
class Inst_r_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                   string asmstr, list<dag> pattern>
    : Inst_rr_short<opclass, opcode, outs, ins, asmstr, pattern> {
  let rA = 0;
}


class Inst_rr_i6<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<6> imm;
  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0} = imm{2-0};
  let Inst{18-16} = imm{5-3};
}
class Inst_rr_i3_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                       string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<3> rA;
  bits<3> imm;
  let Inst{8-6} = rD;
  let Inst{5-3} = rA;
  let Inst{2-0} = imm;
}


class Inst_rr_i9<bits<4> opclass, bits<5> opcode, dag outs, dag ins,
                 string asmstr, list<dag>pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<9> imm;

  let Inst{12-9} = opcode{3-0}; // Define low bits of the opcode field
  let Inst{25} = opcode{4};

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0} = imm{2-0};
  let Inst{18-16} = imm{5-3};
  let Inst{28-26} = imm{8-6};   // Immediate reuses high bits of opcode field
}
class Inst_rr_i10<bits<4> opclass, bits<4> opcode, dag outs, dag ins,
                  string asmstr, list<dag> pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  bits<6> rD;
  bits<6> rA;
  bits<10> imm;

  let Inst{12-9} = opcode;      // Define low bits of the opcode field

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0} = imm{2-0};
  let Inst{18-16} = imm{5-3};
  let Inst{28-25} = imm{9-6};   // Immediate reuses high bits of opcode field
}


class Inst_r_i12<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6>  rD;
  bits<12> imm;

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-0} = imm{5-0};
  let Inst{21-16} = imm{11-6};
}
class Inst_r_i6_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                      string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> rD;
  bits<6> imm;
  let Inst{8-6} = rD;
  let Inst{5-0} = imm;
}


class Inst_r_i16<bits<4> opclass, bits<4> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  bits<6>  rD;
  bits<16> imm;

  let Inst{12-9} = opcode;      // Define low bits of the opcode field

  let Inst{8-6} = rD{2-0};
  let Inst{24-22} = rD{5-3};
  let Inst{5-0} = imm{5-0};
  let Inst{21-16} = imm{11-6};
  let Inst{28-25} = imm{15-12}; // Imm reuses high bits of opcode field
}


class Inst_i18<bits<4> opclass, bits<8> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstAAP<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<18> imm;
  let Inst{8-0} = imm{8-0};
  let Inst{24-16} = imm{17-9};
}
class Inst_i9_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                      string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<9> imm;
  let Inst{8-0} = imm;
}


class Inst_i16_r<bits<4> opclass, bits<4> opcode, dag outs, dag ins,
                 string asmstr, list<dag> pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  bits<16> imm;
  bits<6>  rA;

  let Inst{12-9} = opcode;  // Define low bits of opcode field

  let Inst{2-0} = imm{2-0};
  let Inst{8-6} = imm{5-3};
  let Inst{18-16} = imm{8-6};
  let Inst{28-22} = imm{15-9}; // Reuse opcode high bits

  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
}
class Inst_i6_r_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                      string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<6> imm;
  bits<3> rA;
  let Inst{2-0} = imm{2-0};
  let Inst{8-6} = imm{5-3};
  let Inst{5-3} = rA{2-0};
}


class Inst_i10_rr<bits<4> opclass, bits<4> opcode, dag outs, dag ins,
                  string asmstr, list<dag> pattern>
    : InstAAP_base<opclass, outs, ins, asmstr, pattern> {
  bits<10> imm;
  bits<6>  rA;
  bits<6>  rB;

  let Inst{12-9} = opcode;    // Define low bits of opcode field

  let Inst{8-6} = imm{2-0};
  let Inst{28-22} = imm{9-3}; // Reuse top bits of opcode field
  let Inst{5-3} = rA{2-0};
  let Inst{21-19} = rA{5-3};
  let Inst{2-0} = rB{2-0};
  let Inst{18-16} = rB{5-3};
}
class Inst_i3_rr_short<bits<2> opclass, bits<4> opcode, dag outs, dag ins,
                       string asmstr, list<dag> pattern>
    : InstAAP_short<opclass, opcode, outs, ins, asmstr, pattern> {
  bits<3> imm;
  bits<3> rA;
  bits<3> rB;
  let Inst{8-6} = imm;
  let Inst{5-3} = rA;
  let Inst{2-0} = rB;
}


//===----------------------------------------------------------------------===//
// Pseudo Instruction
//===----------------------------------------------------------------------===//

let isCodeGenOnly = 1 in {
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstAAP<0x0, 0x0, outs, ins, asmstr, pattern> {
  let Inst{31-0} = 0;
}
}

