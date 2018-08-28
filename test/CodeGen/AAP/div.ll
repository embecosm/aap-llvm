; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of expansion of various division calls

define i16 @udiv(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: udiv:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __udivhi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = udiv i16 %a, %b
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @udiv_constant(i16 %a) nounwind {
; CHECK-LABEL: udiv_constant:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         movi $r3, 5
; CHECK:         bal __udivhi3, $r0
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = udiv i16 %a, 5
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @udiv_pow2(i16 %a) nounwind {
; CHECK-LABEL: udiv_pow2:
; CHECK:         lsri $r2, $r2, 3
  %1 = udiv i16 %a, 8
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @udiv32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: udiv32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __udivsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = udiv i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @udiv32_constant(i32 %a) nounwind {
; CHECK-LABEL: udiv32_constant:
; CHECK:         subi $r1, $r1, 6
; CHECK:         stw [$r1, 4], $r0
; CHECK:         stw [$r1, 2], $r4
; CHECK:         stw [$r1, 0], $r5
; CHECK:         movi $r4, 5
; CHECK:         movi $r5, 0
; CHECK:         bal __udivsi3, $r0
; CHECK:         ldw $r5, [$r1, 0]
; CHECK:         ldw $r4, [$r1, 2]
; CHECK:         ldw $r0, [$r1, 4]
; CHECK:         addi $r1, $r1, 6
  %1 = udiv i32 %a, 5
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @sdiv(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: sdiv:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __divhi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = sdiv i16 %a, %b
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @sdiv_constant(i16 %a) nounwind {
; CHECK-LABEL: sdiv_constant:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         movi $r3, 5
; CHECK:         bal __divhi3, $r0
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = sdiv i16 %a, 5
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @sdiv_pow2(i16 %a) nounwind {
; CHECK-LABEL: sdiv_pow2:
; CHECK:         asri $[[REG1:r[0-9]+]], $r2, 15
; CHECK:         lsri $[[REG1]], $[[REG1]], 13
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         asri $r2, $r2, 3
  %1 = sdiv i16 %a, 8
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @sdiv32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: sdiv32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __divsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = sdiv i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @sdiv32_constant(i32 %a) nounwind {
; CHECK-LABEL: sdiv32_constant:
; CHECK:         subi $r1, $r1, 6
; CHECK:         stw [$r1, 4], $r0
; CHECK:         stw [$r1, 2], $r4
; CHECK:         stw [$r1, 0], $r5
; CHECK:         movi $r4, 5
; CHECK:         movi $r5, 0
; CHECK:         bal __divsi3, $r0
; CHECK:         ldw $r5, [$r1, 0]
; CHECK:         ldw $r4, [$r1, 2]
; CHECK:         ldw $r0, [$r1, 4]
; CHECK:         addi $r1, $r1, 6
  %1 = sdiv i32 %a, 5
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}
