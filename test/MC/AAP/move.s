; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; move instructions.

; Moves with registers
mov   $r0,  $r0     ;CHECK: encoding: [0x00,0x12]
mov   $r7,  $r7     ;CHECK: encoding: [0xf8,0x13]
mov   $r8,  $r8     ;CHECK: encoding: [0x00,0x92,0x48,0x00]
mov   $r15, $r1     ;CHECK: encoding: [0xc8,0x93,0x40,0x00]
mov   $r3,  $r11    ;CHECK: encoding: [0xd8,0x92,0x08,0x00]

; Moves with immediates
movi  $r0,  0       ;CHECK: encoding: [0x00,0x1e]
movi  $r5,  -0      ;CHECK: encoding: [0x40,0x1f]
movi  $r7,  +0      ;CHECK: encoding: [0xc0,0x1f]
movi  $r2,  13      ;CHECK: encoding: [0x8d,0x1e]
movi  $r3,  63      ;CHECK: encoding: [0xff,0x1e]
movi  $r3,  0x12    ;CHECK: encoding: [0xd2,0x1e]
movi  $r4,  64      ;CHECK: encoding: [0x00,0x9f,0x01,0x00]
movi  $r10, 0       ;CHECK: encoding: [0x80,0x9e,0x40,0x00]
movi  $r8,  123     ;CHECK: encoding: [0x3b,0x9e,0x41,0x00]
movi  $r9,  65535   ;CHECK: encoding: [0x7f,0x9e,0x7f,0x1e]
movi  $r1,  -1      ;CHECK: encoding: [0x7f,0x9e,0x3f,0x1e]
movi  $r9,  -32768  ;CHECK: encoding: [0x40,0x9e,0x40,0x10]
movi  $r16, 0xdead  ;CHECK: encoding: [0x2d,0x9e,0xba,0x1a]
movi  $r63, -0x1    ;CHECK: encoding: [0xff,0x9f,0xff,0x1f]
movi  $r63, -0x7fff ;CHECK: encoding: [0xc1,0x9f,0xc0,0x11]

; Moves with expressions
movi  $r2,  (5 + 5)           ;CHECK: [0x8a,0x1e]
movi  $r4,  (65 - 10)         ;CHECK: [0x37,0x1f]
movi  $r12, (0xdead - 0xbeef) ;CHECK: [0x3e,0x9f,0x7e,0x02]

; Moves with relocations. These should always assemble to long instructions
movi  $r0,  a       ;CHECK: encoding: [0b00AAAAAA,0x9e,0x00,0x00]
movi  $r8,  b       ;CHECK: encoding: [0b00AAAAAA,0x9e,0x40,0x00]
movi  $r16, (a + b) ;CHECK: encoding: [0b00AAAAAA,0x9e,0x80,0x00]
