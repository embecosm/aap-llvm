//===- AAPCallingConv.td - Calling Conventions for AAP -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the AAP architectures.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

def RetCC_AAP : CallingConv<[
  // Use the same registers for returns as argument passing.
  CCIfType<[i16], CCAssignToReg<[R2, R3, R4, R5, R6, R7]>>
]>;

//===----------------------------------------------------------------------===//
// Argument Calling Conventions
//===----------------------------------------------------------------------===//

def CC_AAP : CallingConv<[
  // Promote i8 args to i16
  CCIfType<[i8], CCPromoteToType<i16>>,

  // All arguments get passed into registers if there is space.
  CCIfType<[i16], CCAssignToReg<[R2, R3, R4, R5, R6, R7]>>,

  // Alternatively they are added to the stack.
  CCAssignToStack<2, 2>
]>;

// The callee saved registers are spread out to ensure that approximately
// two-third are callee-saved even when the number of registers is restricted.
//
// R1 is reserved as the stack pointer
def CSR : CalleeSavedRegs<
  (add R0,  R2,  R3,  R4,  R5,  R6,  R7,  R8,  R9,  R11, R12, R14, R15, R17,
       R18, R20, R21, R23, R24, R26, R27, R29, R30, R32, R34, R36, R38, R40,
       R42, R44, R46, R48, R50, R52, R54, R56, R58, R60, R62)>;
