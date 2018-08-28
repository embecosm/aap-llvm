; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of expanded load extensions and truncations.

define i8 @sext_i1_to_i8(i1 %a) {
; CHECK-LABEL: sext_i1_to_i8:
; CHECK:         andi $r2, $r2, 1
; CHECK:         movi $[[REG1:r[0-9]+]], 0
; CHECK:         sub $r2, $[[REG1]], $r2
  %1 = sext i1 %a to i8
  ret i8 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @sext_i1_to_i16(i1 %a) {
; CHECK-LABEL: sext_i1_to_i16:
; CHECK:         andi $r2, $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], 0
; CHECK:         sub $r2, $[[REG2]], $r2
  %1 = sext i1 %a to i16
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @sext_i8_to_i16(i8 %a) {
; CHECK-LABEL: sext_i8_to_i16:
; CHECK:         lsli $r2, $r2, 8
; CHECK:         asri $r2, $r2, 8
  %1 = sext i8 %a to i16
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i8 @zext_i1_to_i8(i1 %a) {
; CHECK-LABEL: zext_i1_to_i8:
; CHECK:         andi $r2, $r2, 1
  %1 = zext i1 %a to i8
  ret i8 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @zext_i1_to_i16(i1 %a) {
; CHECK-LABEL: zext_i1_to_i16:
; CHECK:         andi $r2, $r2, 1
  %1 = zext i1 %a to i16
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @zext_i8_to_i16(i8 %a) {
; CHECK-LABEL: zext_i8_to_i16:
; CHECK:         andi $r2, $r2, 255
  %1 = zext i8 %a to i16
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i1 @trunc_i8_to_i1(i8 %a) {
; CHECK-LABEL: trunc_i8_to_i1:
  %1 = trunc i8 %a to i1
  ret i1 %1 ; CHECK: jmp   {{.*JMP}}
}

define i1 @trunc_i16_to_i1(i16 %a) {
; CHECK-LABEL: trunc_i16_to_i1:
  %1 = trunc i16 %a to i1
  ret i1 %1 ; CHECK: jmp   {{.*JMP}}
}

define i8 @trunc_i16_to_i8(i16 %a) {
; CHECK-LABEL: trunc_i16_to_i8:
  %1 = trunc i16 %a to i8
  ret i8 %1 ; CHECK: jmp   {{.*JMP}}
}
