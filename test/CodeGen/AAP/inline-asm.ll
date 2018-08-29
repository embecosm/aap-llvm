; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check that basic inline asm constraints can be correctly lowered.

@gi = external global i16

define i16 @constraint_r(i16 %a) {
; CHECK-LABEL: constraint_r:
; CHECK:         movi $[[REG1:r[0-9]+]], gi
; CHECK:         ldw $[[REG1]], [$[[REG1]], 0]
; CHECK:         ;APP
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         ;NO_APP
  %1 = load i16, i16* @gi
  %2 = tail call i16 asm "add $0, $1, $2", "=r,r,r"(i16 %a, i16 %1)
  ret i16 %2 ; CHECK: jmp   {{.*JMP}}
}

define i16 @constraint_i(i16 %a) {
; CHECK-LABEL: constraint_i:
; CHECK:         ;APP
; CHECK:         addi $r2, $r2, 113
; CHECK:         ;NO_APP
  %1 = load i16, i16* @gi
  %2 = tail call i16 asm "addi $0, $1, $2", "=r,r,i"(i16 %a, i16 113)
  ret i16 %2 ; CHECK: jmp   {{.*JMP}}
}

define void @constraint_m(i16* %a) {
; CHECK-LABEL: constraint_m:
; CHECK:         ;APP
; CHECK:         ;NO_APP
  call void asm sideeffect "", "=*m"(i16* %a)
  ret void ; CHECK: jmp   {{.*JMP}}
}

define i16 @constraint_m2(i16* %a) {
; RV32I-LABEL: constraint_m2:
; CHECK:         ;APP
; CHECK:         ldw $r2, [$r2, 0]
; CHECK:         ;NO_APP
  %1 = tail call i16 asm "ldw $0, $1", "=r,*m"(i16* %a) nounwind
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}
