; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; noop instruction.

nop   $r0,  5   ;CHECK: {{NOP_short}}
nop   $r7,  63  ;CHECK: {{NOP_short}}
nop   $r8,  53  ;CHECK: {{NOP$}}
nop   $r4,  64  ;CHECK: {{NOP$}}
nop   $r63, a   ;CHECK: {{NOP$}}
