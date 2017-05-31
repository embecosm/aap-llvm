//===- AAPRegisterInfo.td - AAP Register defs --------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the AAP register file
//===----------------------------------------------------------------------===//

class AAPReg<string n> : Register<n> {
  field bits<6> Num;
  let Namespace = "AAP";
}

// Registers are identified with 5-bit ID numbers.
// Ri - 16-bit integer registers
class Ri<bits<6> num, string n> : AAPReg<n> {
  let Num = num;
  let HWEncoding{5-0} = num;
}

foreach I = 0-63 in
def R#I : Ri<I, "r"#I>, DwarfRegNum<[I]>;

// PSW holds the carry flag
def PSW : AAPReg<"psw">, DwarfRegNum<[64]>;

// Register classes.
def GR8  : RegisterClass<"AAP", [i16], 16, (add (sequence "R%u", 0,  7))>;
def GR64 : RegisterClass<"AAP", [i16], 16, (add (sequence "R%u", 0, 63))>;
