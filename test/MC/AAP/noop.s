; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; noop instruction.

nop   $r0,  5   ;CHECK: encoding: [0x05,0x00]
nop   $r7,  63  ;CHECK: encoding: [0xff,0x01]
nop   $r8,  53  ;CHECK: encoding: [0x35,0x80,0x40,0x00]
nop   $r4,  64  ;CHECK: encoding: [0x00,0x81,0x01,0x00]
nop   $r63, a   ;CHECK: encoding: [0b11AAAAAA,0x81,0xc0,0x01]
