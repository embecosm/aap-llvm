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
  CCIfType<[i16], CCAssignToReg<[R3, R4, R5, R6, R7]>>
]>;

//===----------------------------------------------------------------------===//
// Argument Calling Conventions
//===----------------------------------------------------------------------===//

def CC_AAP : CallingConv<[
  // Promote i8 args to i16
  CCIfType<[i8], CCPromoteToType<i16>>,

  // All arguments get passed into registers if there is space.
  CCIfType<[i16], CCAssignToReg<[R3, R4, R5, R6]>>,

  // Alternatively they are added to the stack.
  CCAssignToStack<2, 2>
]>;

def CSR : CalleeSavedRegs<(add R20, R21, R22, R23, R24, R25, R26, R27, R28,
                           R29, R30, R31)>;
