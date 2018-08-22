; RUN: llvm-mc -triple=aap < %s -show-encoding \
; RUN:     | FileCheck -check-prefix=CHECK-FIXUP %s
; RUN: llvm-mc -filetype=obj -triple=aap < %s \
; RUN:     | llvm-objdump -d -r - \
; RUN:     | FileCheck -check-prefix=CHECK-INSTR %s

; Checks that fixups that can be resolved within the same object file are
; applied correctly

; CHECK-FIXUP: fixup A - offset: 0, value: u16, kind: fixup_AAP_ABS16
; CHECK-INSTR: movi $r2, 65535
movi $r2, u16
; CHECK-FIXUP: fixup A - offset: 0, value: t1+u16, kind: fixup_AAP_ABS16
; CHECK-INSTR: movi $r2, 0
; CHECK-INSTR: R_AAP_ABS16 t1+65535
movi $r2, t1+u16

; CHECK-FIXUP: fixup A - offset: 0, value: i12, kind: fixup_AAP_ABS12
; CHECK-INSTR: nop $r2, 2048
nop $r2, i12
; CHECK-FIXUP: fixup A - offset: 0, value: t1+i12, kind: fixup_AAP_ABS12
; CHECK-INSTR: nop $r2, 0
; CHECK-INSTR: R_AAP_ABS12 t1-2048
nop $r2, t1+i12

; CHECK-FIXUP: fixup A - offset: 0, value: i10, kind: fixup_AAP_ABS10
; CHECK-INSTR: addi $r2, $r2, 512
addi $r2, $r2, i10
; CHECK-FIXUP: fixup A - offset: 0, value: t1+i10, kind: fixup_AAP_ABS10
; CHECK-INSTR: addi $r2, $r2, 0
; CHECK-INSTR: R_AAP_ABS10 t1-512
addi $r2, $r2, t1+i10

; CHECK-FIXUP: fixup A - offset: 0, value: i10, kind: fixup_AAP_OFF10
; CHECK-INSTR: stw [$r3, -512], $r2
stw [$r3, i10], $r2
; CHECK-FIXUP: fixup A - offset: 0, value: t1+i10, kind: fixup_AAP_OFF10
; CHECK-INSTR: stw [$r3, 0], $r2
; CHECK-INSTR: R_AAP_OFF10 t1-512
stw [$r3, t1+i10], $r2

; CHECK-FIXUP: fixup A - offset: 0, value: i9, kind: fixup_AAP_ABS9
; CHECK-INSTR: andi $r2, $r2, 256
andi $r2, $r2, i9
; CHECK-FIXUP: fixup A - offset: 0, value: t1+i9, kind: fixup_AAP_ABS9
; CHECK-INSTR: andi $r2, $r2, 0
; CHECK-INSTR: R_AAP_ABS9 t1-256
andi $r2, $r2, t1+i9

; CHECK-FIXUP: fixup A - offset: 0, value: u6, kind: fixup_AAP_SHIFT6
; CHECK-INSTR: lsli $r2, $r2, 64
lsli $r2, $r2, u6
; CHECK-FIXUP: fixup A - offset: 0, value: t1+u6, kind: fixup_AAP_SHIFT6
; CHECK-INSTR: lsli $r2, $r2, 1
; CHECK-INSTR: R_AAP_SHIFT6 t1+63
lsli $r2, $r2, t1+u6

.fill 976

.LBB0:
; CHECK-FIXUP: fixup A - offset: 0, value: .LBB0, kind: fixup_AAP_BAL32
; CHECK-INSTR: bal 0, $r0
; CHECK-INSTR: R_AAP_BAL32 .text+1024
bal .LBB0, $r0

; CHECK-FIXUP: fixup A - offset: 0, value: .LBB0, kind: fixup_AAP_BRCC32
; CHECK-INSTR: beq 0, $r2, $r3
; CHECK-INSTR: R_AAP_BRCC32 .text+1024
beq .LBB0, $r2, $r3

; CHECK-FIXUP: fixup A - offset: 0, value: .LBB0, kind: fixup_AAP_BR32
; CHECK-INSTR: bra 0
; CHECK-INSTR: R_AAP_BR32 .text+1024
bra .LBB0

.set u16, 65535
.set i12, -2048
.set i10, -512
.set i9, -256
.set u6, 63