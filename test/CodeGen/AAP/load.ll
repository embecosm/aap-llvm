; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of various simple load operations.


@a = external global i16
@b = external global i16

@c = external global i8
@d = external global i8


define i8 @ldb_global() {
entry:
;CHECK: ldb_global:
;CHECK: movi $[[REG1:r[0-9]+]], c             {{.*MOVI_i16}}
;CHECK: ldb ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDB}}
  %0 = load i8, i8* @c, align 1
  ret i8 %0 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_imm() {
entry:
;CHECK: ldb_imm:
;CHECK: movi $[[REG1:r[0-9]+]], 123           {{.*MOVI_i16}}
;CHECK: ldb ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDB}}
  %0 = inttoptr i16 123 to i8*
  %1 = load i8, i8* %0, align 1
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_reg(i8* %x) {
entry:
;CHECK: ldb_reg:
;CHECK: ldb ${{r[0-9]+}}, [${{r[0-9]+}}, 0]   {{.*LDB}}
  %0 = load i8, i8* %x, align 1
  ret i8 %0 ;CHECK: jmp   {{.*JMP}}
}




define i16 @ldw_global() {
entry:
;CHECK: ldw_global:
;CHECK: movi $[[REG1:r[0-9]+]], a             {{.*MOVI_i16}}
;CHECK: ldw ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDW}}
  %0 = load i16, i16* @a, align 2
  ret i16 %0 ;CHECK: jmp   {{.*JMP}}
}

define i16 @ldw_imm() {
entry:
;CHECK: ldw_imm:
;CHECK: movi $[[REG1:r[0-9]+]], 123           {{.*MOVI_i16}}
;CHECK: ldw ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDW}}
  %0 = inttoptr i16 123 to i16*
  %1 = load i16, i16* %0, align 2
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}

define i16 @ldw_reg(i16* %x) {
entry:
;CHECK: ldw_reg:
;CHECK: ldw ${{r[0-9]+}}, [${{r[0-9]+}}, 0]   {{.*LDW}}
  %0 = load i16, i16* %x, align 2
  ret i16 %0 ;CHECK: jmp   {{.*JMP}}
}
