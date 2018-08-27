; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of loads with offsets


; Byte loads from a register with an immediate offset

define i8 @ldb_reg_imm(i8* %x) {
entry:
;CHECK: ldb_reg_imm:
;CHECK-DAG: ldb ${{r[0-9]+}}, [${{r[0-9]+}}, 15]  {{.*LDB}}
  %0 = ptrtoint i8* %x to i16
  %1 = add i16 %0, 15
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_reg_big_imm(i8* %x) {
entry:
;CHECK: ldb_reg_big_imm:
;CHECK movi ${{r[0-9]+}}, 12345               {{.*MOVI_i16}}
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*ADD_r}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB}}
  %0 = ptrtoint i8* %x to i16
  %1 = add i16 %0, 12345
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}

; With a negative offset
define i8 @ldb_reg_neg_imm(i8* %x) {
entry:
;CHECK: ldb_reg_neg_imm:
;CHECK: ldb ${{r[0-9]+}}, [${{r[0-9]+}}, -432]  {{.*LDB$}}
  %0 = ptrtoint i8* %x to i16
  %1 = sub i16 %0, 432
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}


; TODO: globals
; TODO: postinc/predec?


; Word loads from a register with an immediate offset

define i16 @ldw_reg_imm(i16* %x) {
entry:
;CHECK: ldw_reg_imm:
;CHECK-DAG: ldw ${{r[0-9]+}}, [${{r[0-9]+}}, 16]  {{.*LDW}}
  %0 = ptrtoint i16* %x to i16
  %1 = add i16 %0, 16
  %2 = inttoptr i16 %1 to i16*
  %3 = load i16, i16* %2
  ret i16 %3 ;CHECK: jmp   {{.*JMP}}
}

define i16 @ldw_reg_big_neg_imm(i16* %x) {
entry:
;CHECK: ldw_reg_big_neg_imm:
;CHECK movi ${{r[0-9]+}}, 22222               {{.*MOVI_i16}}
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDW}}
  %0 = ptrtoint i16* %x to i16
  %1 = sub i16 %0, 22222
  %2 = inttoptr i16 %1 to i16*
  %3 = load i16, i16* %2
  ret i16 %3 ;CHECK: jmp   {{.*JMP}}
}


; TODO: globals
; TODO: postinc/predec?
