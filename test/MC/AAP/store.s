; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; store instructions.

; Byte stores with immediates
stb   [$r0,   0],   $r0   ;CHECK: encoding: [0x00,0x30]
stb   [$r1,   2],   $r1   ;CHECK: encoding: [0x4a,0x30]
stb   [$r7,   3],   $r3   ;CHECK: encoding: [0xdb,0x31]
stb   [$r1,  -1],   $r7   ;CHECK: encoding: [0x7f,0x30]
stb   [$r5,  -4],   $r5   ;CHECK: encoding: [0x6c,0x31]
stb   [$r8,   0],   $r0   ;CHECK: encoding: [0x00,0xb0,0x40,0x00]
stb   [$r1,   4],   $r0   ;CHECK: encoding: [0x44,0xb0,0x00,0x00] 
stb   [$r7,   3],   $r8   ;CHECK: encoding: [0xc3,0xb1,0x08,0x00]
stb   [$r15, -1],   $r7   ;CHECK: encoding: [0xff,0xb1,0x47,0x1e]
stb   [$r6,  -5],   $r2   ;CHECK: encoding: [0x93,0xb1,0x07,0x1e]
stb   [$r63,  2],   $r7   ;CHECK: encoding: [0xfa,0xb1,0xc0,0x01]
stb   [$r5,   1],   $r26  ;CHECK: encoding: [0x51,0xb1,0x18,0x00]
stb   [$r63,  511], $r19  ;CHECK: encoding: [0xdf,0xb1,0xd7,0x0f]
stb   [$r52, -512], $r14  ;CHECK: encoding: [0x30,0xb1,0x88,0x11]

; Byte stores with expressions
stb   [$r1,   (-(3 - 2))],        $r2   ;CHECK: encoding: [0x57,0x30]
stb   [$r4,   (1234 - 1233)],     $r7   ;CHECK: encoding: [0x39,0x31]
stb   [$r6,   (-2 + 1)],          $r2   ;CHECK: encoding: [0x97,0x31]
stb   [$r1,   (1024 - 513)],      $r0   ;CHECK: encoding: [0x47,0xb0,0x07,0x0e]
stb   [$r40,  (-256 + 123)],      $r16  ;CHECK: encoding: [0x03,0xb0,0x57,0x1b]
stb   [$r18,  (0xdead - 0xdeaf)], $r8   ;CHECK: encoding: [0x86,0xb0,0x8f,0x1e]

; Byte stores with relocations
stb   [$r0,  a],        $r1   ;CHECK: encoding: [0b00001AAA,0xb0,0x00,0x00]
stb   [$r7,  b],        $r2   ;CHECK: encoding: [0b11010AAA,0xb1,0x00,0x00]
stb   [$r1,  (c - a)],  $r50  ;CHECK: encoding: [0b01010AAA,0xb0,0x30,0x00]

; Postinc/Predec byte stores with immediates
stb   [$r5+,   1],   $r0   ;CHECK: encoding: [0x41,0x33]
stb   [$r6+,   3],   $r2   ;CHECK: encoding: [0x93,0x33]
stb   [-$r7,   2],   $r7   ;CHECK: encoding: [0xfa,0x35]
stb   [-$r1,  -1],   $r7   ;CHECK: encoding: [0x7f,0x34]
stb   [$r2+,  -4],   $r2   ;CHECK: encoding: [0x94,0x32]
stb   [$r9+,   1],   $r8   ;CHECK: encoding: [0x41,0xb2,0x48,0x00]
stb   [$r1+,   4],   $r0   ;CHECK: encoding: [0x44,0xb2,0x00,0x00]
stb   [-$r7,  -5],   $r8   ;CHECK: encoding: [0xc3,0xb5,0x0f,0x1e]
stb   [-$r12, -1],   $r7   ;CHECK: encoding: [0x3f,0xb5,0x47,0x1e]
stb   [-$r61,  -5],  $r2   ;CHECK: encoding: [0x53,0xb5,0xc7,0x1f]
stb   [$r25+,  2],   $r7   ;CHECK: encoding: [0x7a,0xb2,0xc0,0x00]
stb   [$r1+,   1],   $r29  ;CHECK: encoding: [0x69,0xb2,0x18,0x00]
stb   [-$r27,  511], $r19  ;CHECK: encoding: [0xdf,0xb4,0xd7,0x0e]
stb   [$r52+, -512], $r44  ;CHECK: encoding: [0x20,0xb3,0xa8,0x11]

; Postinc/Predec byte stores with expressions
stb   [$r0+,   (-(-5 + 4))],       $r0   ;CHECK: encoding: [0x01,0x32]
stb   [-$r2,   (91 - 95)],         $r2   ;CHECK: encoding: [0x94,0x34]
stb   [$r6+,   (2 - 1)],           $r2   ;CHECK: encoding: [0x91,0x33]
stb   [-$r1,   (1024 - 513)],      $r8   ;CHECK: encoding: [0x47,0xb4,0x0f,0x0e]
stb   [$r40+,  (-256 + 256)],      $r16  ;CHECK: encoding: [0x00,0xb2,0x50,0x01]
stb   [-$r18,  (0xdead - 0xdead)], $r8   ;CHECK: encoding: [0x80,0xb4,0x88,0x00]

; Postinc/Predec byte stores with relocations
stb   [$r7+,  a],        $r6   ;CHECK: encoding: [0b11110AAA,0xb3,0x00,0x00]
stb   [-$r0,  b],        $r3   ;CHECK: encoding: [0b00011AAA,0xb4,0x00,0x00]
stb   [$r1+,  (b + a)],  $r8   ;CHECK: encoding: [0b01000AAA,0xb2,0x08,0x00]


; Word stores with immediates
stw   [$r1,   2],   $r1   ;CHECK: encoding: [0x4a,0x38]
stw   [$r7,   3],   $r3   ;CHECK: encoding: [0xdb,0x39]
stw   [$r8,   0],   $r0   ;CHECK: encoding: [0x00,0xb8,0x40,0x00]
stw   [$r6,  -5],   $r2   ;CHECK: encoding: [0x93,0xb9,0x07,0x1e]
stw   [$r5,   1],   $r26  ;CHECK: encoding: [0x51,0xb9,0x18,0x00]

; Word stores with expressions
stw   [$r4,   (1234 - 1233)],     $r7   ;CHECK: encoding: [0x39,0x39]
stw   [$r6,   (-2 + 1)],          $r2   ;CHECK: encoding: [0x97,0x39]
stw   [$r1,   (1024 - 513)],      $r0   ;CHECK: encoding: [0x47,0xb8,0x07,0x0e]
stw   [$r18,  (0xdead - 0xdeaf)], $r8   ;CHECK: encoding: [0x86,0xb8,0x8f,0x1e]

; Word stores with relocations
stw   [$r7,  b],        $r2   ;CHECK: encoding: [0b11010AAA,0xb9,0x00,0x00]
stw   [$r1,  (c - a)],  $r50  ;CHECK: encoding: [0b01010AAA,0xb8,0x30,0x00]

; Postinc/Predec word stores with immediates
stw   [-$r7,   2],   $r7   ;CHECK: encoding: [0xfa,0x3d]
stw   [$r2+,  -4],   $r2   ;CHECK: encoding: [0x94,0x3a]
stw   [$r9+,   1],   $r8   ;CHECK: encoding: [0x41,0xba,0x48,0x00]
stw   [-$r7,  -5],   $r8   ;CHECK: encoding: [0xc3,0xbd,0x0f,0x1e]
stw   [-$r61,  -5],  $r2   ;CHECK: encoding: [0x53,0xbd,0xc7,0x1f]
stw   [$r25+,  2],   $r7   ;CHECK: encoding: [0x7a,0xba,0xc0,0x00]

; Postinc/Predec word stores with expressions
stw   [-$r2,   (91 - 95)],         $r2   ;CHECK: encoding: [0x94,0x3c]
stw   [$r6+,   (2 - 1)],           $r2   ;CHECK: encoding: [0x91,0x3b]
stw   [$r40+,  (-256 + 256)],      $r16  ;CHECK: encoding: [0x00,0xba,0x50,0x01]
stw   [-$r18,  (0xdead - 0xdead)], $r8   ;CHECK: encoding: [0x80,0xbc,0x88,0x00]

; Postinc/Predec word stores with relocations
stw   [$r7+,  a],        $r6   ;CHECK: encoding: [0b11110AAA,0xbb,0x00,0x00]
stw   [-$r0,  b],        $r3   ;CHECK: encoding: [0b00011AAA,0xbc,0x00,0x00]
