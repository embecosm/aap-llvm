; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of expansion of remainder calls

define i16 @urem(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: urem:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __umodhi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = urem i16 %a, %b
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @srem(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: srem:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal __modhi3, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = srem i16 %a, %b
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}
