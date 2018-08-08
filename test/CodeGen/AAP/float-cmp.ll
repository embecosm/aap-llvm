; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Simple tests of adherence to ABI for floating point comparisons


; EQ

define i16 @float_eq(float %a, float %b) {
entry:
;CHECK:     float_eq:
;CHECK:       bal __eqsf2
;CHECK-NOT:   or $r2, $r3
;CHECK:       mov $[[RES:r[0-9]+]], $r2
;CHECK:       movi $r2, 1
;CHECK:       beq .LBB0_2, $[[RES]], ${{r[0-9]+}}
;CHECK:       movi $r2, 0
;CHECK:     .LBB0_2:
  %cmp = fcmp oeq float %a, %b
  %conv1 = zext i1 %cmp to i16
  ret i16 %conv1 ;CHECK: jmp {{.*JMP}}
}

define i16 @double_eq(double %a, double %b) {
entry:
;CHECK:     double_eq:
;CHECK:       bal __eqdf2
;CHECK-NOT:   or $r2, $r3
;CHECK:       mov $[[RES:r[0-9]+]], $r2
;CHECK:       movi $r2, 1
;CHECK:       beq .LBB1_2, $[[RES]], ${{r[0-9]+}}
;CHECK:       movi $r2, 0
;CHECK:     .LBB1_2:
  %cmp = fcmp oeq double %a, %b
  %conv1 = zext i1 %cmp to i16
  ret i16 %conv1 ;CHECK: jmp {{.*JMP}}
}

; LT

define i16 @float_lt(float %a, float %b) {
entry:
;CHECK:     float_lt:
;CHECK:       bal __ltsf2
;CHECK-NOT:   or $r2, $r3
;CHECK:       mov $[[RES:r[0-9]+]], $r2
;CHECK:       movi $r2, 1
;CHECK:       blts .LBB2_2, $[[RES]], ${{r[0-9]+}}
;CHECK:       movi $r2, 0
;CHECK:     .LBB2_2:
  %cmp = fcmp olt float %a, %b
  %conv1 = zext i1 %cmp to i16
  ret i16 %conv1 ;CHECK: jmp {{.*JMP}}
}

define i16 @double_lt(double %a, double %b) {
entry:
;CHECK:     double_lt:
;CHECK:       bal __ltdf2
;CHECK-NOT:   or $r2, $r3
;CHECK:       mov $[[RES:r[0-9]+]], $r2
;CHECK:       movi $r2, 1
;CHECK:       blts .LBB3_2, $[[RES]], ${{r[0-9]+}}
;CHECK:       movi $r2, 0
;CHECK:     .LBB3_2:
  %cmp = fcmp olt double %a, %b
  %conv1 = zext i1 %cmp to i16
  ret i16 %conv1 ;CHECK: jmp {{.*JMP}}
}
