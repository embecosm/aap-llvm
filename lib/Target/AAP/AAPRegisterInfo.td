//===- AAPRegisterInfo.td - AAP Register defs ------------*- tablegen -*-===//
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
  field bits<5> Num;
  let Namespace = "AAP";
}

// Registers are identified with 5-bit ID numbers.
// Ri - 16-bit integer registers
class Ri<bits<5> num, string n> : AAPReg<n> {
  let Num = num;
  let HWEncoding{4-0} = num;
}

foreach I = 0-63 in
def R#I : Ri<I, "r"#I>, DwarfRegNum<[I]>;

// PSW holds the carry flag
def PSW : AAPReg<"psw">, DwarfRegNum<[64]>;


// Register classes.
def GR8 : RegisterClass<"AAP", [i16], 16,
  (add R0, R1, R2, R3, R4, R5, R6, R7)>;

def GR64 : RegisterClass<"AAP", [i16], 16,
  (add R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,  R8,  R9,  R10, R11, R12,
       R13, R14, R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, R25,
       R26, R27, R28, R29, R30, R31, R32, R33, R34, R35, R36, R37, R38,
       R39, R40, R41, R42, R43, R44, R45, R46, R47, R48, R49, R50, R51,
       R52, R53, R54, R55, R56, R57, R58, R59, R60, R61, R62, R63)>;
