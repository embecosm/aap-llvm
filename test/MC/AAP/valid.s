; RUN: llvm-mc %s -triple=aap -show-encoding \
; RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s

; Check parsing, printing and encoding of valid instructions.

; CHECK-INST: addi $r2, $r2, 0
; CHECK: encoding: [0x90,0x14]
addi $r2, $r2, 0

; CHECK-INST: addi $r2, $r2, 1023
; CHECK: encoding: [0x97,0x94,0x07,0x1e]
addi $r2, $r2, 1023

; CHECK-INST: add $r2, $r2, $r3
; CHECK: encoding: [0x93,0x02]
add $r2, $r2, $r3

; CHECK-INST: add $r15, $r15, $r31
; CHECK: encoding: [0xff,0x83,0x4b,0x00]
add $r15, $r15, $r31

; CHECK-INST: movi $r2, 0
; CHECK: encoding: [0x80,0x1e]
movi $r2, 0

; CHECK-INST: movi $r2, 65535
; CHECK: encoding: [0xbf,0x9e,0x3f,0x1e]
movi $r2, 65535

; CHECK-INST: movi $r2, -32768
; CHECK: encoding: [0x80,0x9e,0x00,0x10]
movi $r2, -32768

; CHECK-INST: mov $r2, $r3
; CHECK: encoding: [0x98,0x12]
mov $r2, $r3

; CHECK-INST: mov $r15, $r31
; CHECK: encoding: [0xf8,0x93,0x58,0x00]
mov $r15, $r31

; CHECK-INST: ldw $r2, [$r3, 0]
; CHECK: encoding: [0x98,0x28]
ldw $r2, [$r3, 0]

ldw $r2, [-$r3, 0]

ldw $r2, [$r3+, 0]

; CHECK-INST: ldw $r2, [$r3, 511]
; CHECK: encoding: [0x9f,0xa8,0x07,0x0e]
ldw $r2, [$r3, 511]

; CHECK-INST: ldw $r2, [$r3, -512]
; CHECK: encoding: [0x98,0xa8,0x00,0x10]
ldw $r2, [$r3, -512]

; CHECK-INST: .L1:
.L1:

; CHECK-INST: bra .L1
; CHECK: encoding: [0x00,0xc0,0x00,0x00]
bra .L1

; CHECK-INST: beq .L1, $r2, $r3
; CHECK: encoding: [0x13,0xc4,0x00,0x00]
beq .L1, $r2, $r3

; CHECK-INST: jmp $r0
; CHECK: encoding: [0x00,0x50]
jmp $r0

; CHECK-INST: jmp $r15
; CHECK: encoding: [0xc0,0xd1,0x40,0x00]
jmp $r15
