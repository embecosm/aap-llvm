; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check that adds and subs with carry are generated properly

define i32 @addcarry(i32 %a, i32 %b) {
; CHECK-LABEL: addcarry:
; CHECK:         add $r2, $r2, $r4
; CHECK:         addc $r3, $r3, $r5
  %1 = add i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @subcarry(i32 %a, i32 %b) {
; CHECK-LABEL: subcarry:
; CHECK:         sub $r2, $r2, $r4
; CHECK:         subc $r3, $r3, $r5
  %1 = sub i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i64 @addcarry64(i64 %a, i64 %b) {
; CHECK-LABEL: addcarry64:
; CHECK:         ldw $[[ARGB3:r[0-9]+]], [$r1, 0]
; CHECK:         ldw $[[ARGB4:r[0-9]+]], [$r1, 2]
; CHECK:         add $r2, $r2, $r6
; CHECK:         addc $r3, $r3, $r7
; CHECK:         addc $r4, $r4, $[[ARGB3]]
; CHECK:         addc $r5, $r5, $[[ARGB4]]
  %1 = add i64 %a, %b
  ret i64 %1 ; CHECK: jmp   {{.*JMP}}
}

define i64 @subcarry64(i64 %a, i64 %b) {
; CHECK-LABEL: subcarry64:
; CHECK:         ldw $[[ARGB3:r[0-9]+]], [$r1, 0]
; CHECK:         ldw $[[ARGB4:r[0-9]+]], [$r1, 2]
; CHECK:         sub $r2, $r2, $r6
; CHECK:         subc $r3, $r3, $r7
; CHECK:         subc $r4, $r4, $[[ARGB3]]
; CHECK:         subc $r5, $r5, $[[ARGB4]]
  %1 = sub i64 %a, %b
  ret i64 %1 ; CHECK: jmp   {{.*JMP}}
}
