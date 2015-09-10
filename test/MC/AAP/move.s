; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; move instructions.

; Moves with registers
mov   $r0,  $r0     ;CHECK: {{MOV_r_short}}
mov   $r7,  $r7     ;CHECK: {{MOV_r_short}}
mov   $r8,  $r8     ;CHECK: {{MOV_r$}}
mov   $r15, $r1     ;CHECK: {{MOV_r$}}
mov   $r3,  $r11    ;CHECK: {{MOV_r$}}

; Moves with immediates
movi  $r0,  0       ;CHECK: {{MOVI_i6_short}}
movi  $r5,  -0      ;CHECK: {{MOVI_i6_short}}
movi  $r7,  +0      ;CHECK: {{MOVI_i6_short}}
movi  $r2,  13      ;CHECK: {{MOVI_i6_short}}
movi  $r3,  63      ;CHECK: {{MOVI_i6_short}}
movi  $r3,  0x12    ;CHECK: {{MOVI_i6_short}}
movi  $r4,  64      ;CHECK: {{MOVI_i16$}}
movi  $r10, 0       ;CHECK: {{MOVI_i16$}}
movi  $r8,  123     ;CHECK: {{MOVI_i16$}}
movi  $r9,  65535   ;CHECK: {{MOVI_i16$}}
movi  $r1,  -1      ;CHECK: {{MOVI_i16$}}
movi  $r9,  -32768  ;CHECK: {{MOVI_i16$}}
movi  $r16, 0xdead  ;CHECK: {{MOVI_i16$}}
movi  $r63, -0x1    ;CHECK: {{MOVI_i16$}}
movi  $r63, -0x7fff ;CHECK: {{MOVI_i16$}}

; Moves with expressions
movi  $r2,  (5 + 5)           ;CHECK: {{MOVI_i6_short}}
movi  $r4,  (65 - 10)         ;CHECK: {{MOVI_i6_short}}
movi  $r12, (0xdead - 0xbeef) ;CHECK: {{MOVI_i16$}}

; Moves with relocations. These should always assemble to long instructions
movi  $r0,  a       ;CHECK: {{MOVI_i16$}}
movi  $r8,  b       ;CHECK: {{MOVI_i16$}}
movi  $r16, (a + b) ;CHECK: {{MOVI_i16$}}
