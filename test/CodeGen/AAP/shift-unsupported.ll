; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of expanded unsupported shift operations

define i32 @lshr32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: lshr32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __lshrsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = lshr i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @ashr32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: ashr32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __ashrsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = ashr i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @shl32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: shl32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __ashlsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = shl i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}
