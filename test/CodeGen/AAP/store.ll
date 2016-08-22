; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of various simple store operations.


@a = external global i16
@b = external global i16

@c = external global i8
@d = external global i8


define void @stb_global_global() {
entry:
;CHECK: stb_global_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], c         {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], d         {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], $[[REG2]]         {{.*STB}}
  %0 = ptrtoint i8* @d to i8
  store i8 %0, i8* @c, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_global_imm() {
entry:
;CHECK: stb_global_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], c         {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], $[[REG2]]         {{.*STB}}
  store i8 123, i8* @c, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_global_reg(i8 %x) {
entry:
;CHECK: stb_global_reg:
;CHECK: movi $[[REG1:r[0-9]+]], c             {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], ${{r[0-9]+}}      {{.*STB}}
  store i8 %x, i8* @c, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_imm_global() {
entry:
;CHECK: stb_imm_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 345       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], c         {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], $[[REG2]]         {{.*STB}}
  %0 = inttoptr i16 345 to i8*
  %1 = ptrtoint i8* @c to i8
  store i8 %1, i8* %0, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_imm_imm() {
entry:
;CHECK: stb_imm_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 456       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], $[[REG2]]         {{.*STB}}
  %0 = inttoptr i16 456 to i8*
  store i8 123, i8* %0, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_imm_reg(i8 %x) {
entry:
;CHECK: stb_imm_reg:
;CHECK: movi $[[REG1:r[0-9]+]], 345           {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], ${{r[0-9]+}}      {{.*STB}}
  %0 = inttoptr i16 345 to i8*
  store i8 %x, i8* %0, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_reg_global(i8 *%x) {
entry:
;CHECK: stb_reg_global:
;CHECK: movi $[[REG1:r[0-9]+]], c             {{.*MOVI_i16}}
;CHECK: stb [${{r[0-9]+}}, 0], $[[REG1]]      {{.*STB}}
  %0 = ptrtoint i8* @c to i8
  store i8 %0, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_reg_imm(i8* %x) {
entry:
;CHECK: stb_reg_imm:
;CHECK: movi $[[REG1:r[0-9]+]], 123           {{.*MOVI_i16}}
;CHECK: stb [${{r[0-9]+}}, 0], $[[REG1]]      {{.*STB}}
  store i8 123, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stb_reg_reg(i8* %x, i8 %y) {
entry:
;CHECK: stb_reg_reg:
;CHECK: stb [${{r[0-9]+}}, 0], ${{r[0-9]+}}   {{.*STB}}
  store i8 %y, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}




define void @stw_global_global() {
entry:
;CHECK: stw_global_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], a         {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], b         {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], $[[REG2]]         {{.*STW}}
  %0 = ptrtoint i16* @b to i16
  store i16 %0, i16* @a, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_global_imm() {
entry:
;CHECK: stw_global_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], a         {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], $[[REG2]]         {{.*STW}}
  store i16 123, i16* @a, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_global_reg(i16 %x) {
entry:
;CHECK: stw_global_reg:
;CHECK: movi $[[REG1:r[0-9]+]], a             {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], ${{r[0-9]+}}      {{.*STW}}
  store i16 %x, i16* @a, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_imm_global() {
entry:
;CHECK: stw_imm_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 345       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], a         {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], $[[REG2]]         {{.*STW}}
  %0 = inttoptr i16 345 to i16*
  %1 = ptrtoint i16* @a to i16
  store i16 %1, i16* %0, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_imm_imm() {
entry:
;CHECK: stw_imm_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 456       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], $[[REG2]]         {{.*STW}}
  %0 = inttoptr i16 456 to i16*
  store i16 123, i16* %0, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_imm_reg(i16 %x) {
entry:
;CHECK: stw_imm_reg:
;CHECK: movi $[[REG1:r[0-9]+]], 345           {{.*MOVI_i16}}
;CHECK: stw [$[[REG1]], 0], ${{r[0-9]+}}      {{.*STW}}
  %0 = inttoptr i16 345 to i16*
  store i16 %x, i16* %0, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_reg_global(i16 *%x) {
entry:
;CHECK: stw_reg_global:
;CHECK: movi $[[REG1:r[0-9]+]], a             {{.*MOVI_i16}}
;CHECK: stw [${{r[0-9]+}}, 0], $[[REG1]]      {{.*STW}}
  %0 = ptrtoint i16* @a to i16
  store i16 %0, i16* %x, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_reg_imm(i16* %x) {
entry:
;CHECK: stw_reg_imm:
;CHECK: movi $[[REG1:r[0-9]+]], 123           {{.*MOVI_i16}}
;CHECK: stw [${{r[0-9]+}}, 0], $[[REG1]]      {{.*STW}}
  store i16 123, i16* %x, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @stw_reg_reg(i16* %x, i16 %y) {
entry:
;CHECK: stw_reg_reg:
;CHECK: stw [${{r[0-9]+}}, 0], ${{r[0-9]+}}   {{.*STW}}
  store i16 %y, i16* %x, align 2
  ret void ;CHECK: jmp    {{.*JMP}}
}
