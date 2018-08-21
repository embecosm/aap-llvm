; RUN: not llvm-mc %s -triple=aap 2>&1 | FileCheck %s

; Check correct detection of invalid instructions.

; CHECK: error: Invalid operand for instruction
addi $r2, $r2, 1024

; CHECK: error: Invalid operand for instruction
addi $r2, 0, $r2

; CHECK: error: Invalid operand for instruction
movi $r2, 65536

; CHECK: error: Invalid operand for instruction
movi $r2, -32769

; CHECK: error: Invalid operand for instruction
movi 0, $r2

; CHECK: error: Invalid operand for instruction
ldw $r2, [$r3, 512]

; CHECK: error: Invalid operand for instruction
ldw $r2, [$r3, -513]

; CHECK: error: Invalid operand for instruction
ldw $r2, [-$r3+, 0]

.L1:

; CHECK: error: Invalid operand for instruction
beq $r2, .L1, $r3
