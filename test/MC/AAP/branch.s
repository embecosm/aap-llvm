; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shorest)
; branch instructions
; Short branch instructions are never assembled

beq    511, $r0,  $r1   ;CHECK: beq   511, $r0,  $r1  ; encoding: [0xc1,0xc5,0xc0,0x0f]
bne   -512, $r7,  $r0   ;CHECK: bne  -512, $r7,  $r0  ; encoding: [0x38,0xc6,0x00,0x10]
blts    15, $r13, $r61  ;CHECK: blts   15, $r13, $r61 ; encoding: [0xed,0xc9,0x4f,0x00]
bles    32, $r9,  $r8   ;CHECK: bles   32, $r9,  $r8  ; encoding: [0x08,0xca,0x09,0x01]
bltu   186, $r11, $r12  ;CHECK: bltu  186, $r11, $r12 ; encoding: [0x9c,0xcc,0xc9,0x05]
bleu    -1, $r17, $r19  ;CHECK: bleu   -1, $r17, $r19 ; encoding: [0xcb,0xcf,0xd2,0x1f]

bra      123    ;CHECK: bra     123 ; encoding: [0x7b,0xc0,0x00,0x00]
bra    -6810    ;CHECK: bra   -6810 ; encoding: [0x66,0xc1,0xf2,0x1f]
bra  -131072    ;CHECK: bra -131072 ; encoding: [0x00,0xc0,0x00,0x1f]
bra   131071    ;CHECK: bra  131071 ; encoding: [0xff,0xc1,0xff,0x00]

bal      481, $r14    ;CHECK: bal    481, $r14 ; encoding: [0x0e,0xc3,0x39,0x00]
bal    -1892, $r58    ;CHECK: bal  -1892, $r58 ; encoding: [0xe2,0xc2,0x17,0x1f]
bal    32767, $r0     ;CHECK: bal  32767, $r0  ; encoding: [0xf8,0xc3,0xf8,0x0f]
bal   -32768, $r15    ;CHECK: bal -32768, $r15 ; encoding: [0x07,0xc2,0x01,0x10]
