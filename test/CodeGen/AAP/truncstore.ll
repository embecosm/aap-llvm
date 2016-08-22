; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of truncstore operations in codegen.


@c = external global i8
@d = external global i8


define void @truncstore_i16_i8_global_global() {
entry:
;CHECK: truncstore_i16_i8_global_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], c       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], d       {{.*MOVI_i16}}
;CHECK: stb [$[[REG2]], 0], $[[REG1]]       {{.*STB}}
  %0 = ptrtoint i8* @c to i16
  %1 = trunc i16 %0 to i8
  store i8 %1, i8* @d, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_global_imm() {
entry:
;CHECK: truncstore_i16_i8_global_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 210     {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], c       {{.*MOVI_i16}}
;CHECK: stb [$[[REG2]], 0], $[[REG1]]       {{.*STB}}
  %0 = trunc i16 1234 to i8
  store i8 %0, i8* @c, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_global_reg(i16 %x) {
entry:
;CHECK: truncstore_i16_i8_global_reg:
;CHECK: movi $[[REG2:r[0-9]+]], c           {{.*MOVI_i16}}
;CHECK: stb [$[[REG2]], 0], ${{r[0-9]+}}    {{.*STB(_short)?}}
  %0 = trunc i16 %x to i8
  store i8 %0, i8* @c, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_imm_global() {
entry:
;CHECK: truncstore_i16_i8_imm_global:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], c       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 1234    {{.*MOVI_i16}}
;CHECK: stb [$[[REG2]], 0], $[[REG1]]       {{.*STB(_short)?}}
  %0 = ptrtoint i8* @c to i16
  %1 = trunc i16 %0 to i8
  %2 = inttoptr i16 1234 to i8*
  store i8 %1, i8* %2, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_imm_imm() {
entry:
;CHECK: truncstore_i16_i8_imm_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 215     {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 1234    {{.*MOVI_i16}}
;CHECK: stb [$[[REG2]], 0], $[[REG1]]       {{.*STB}}
  %0 = trunc i16 4567 to i8
  %1 = inttoptr i16 1234 to i8*
  store i8 %0, i8* %1, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_imm_reg(i16 %x) {
entry:
;CHECK: truncstore_i16_i8_imm_reg:
;CHECK: movi $[[REG1:r[0-9]+]], 1234        {{.*MOVI_i16}}
;CHECK: stb [$[[REG1]], 0], ${{r[0-9]+}}    {{.*STB}}
  %0 = trunc i16 %x to i8
  %1 = inttoptr i16 1234 to i8*
  store i8 %0, i8* %1, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_reg_global(i8* %x) {
entry:
;CHECK: truncstore_i16_i8_reg_global:
;CHECK: movi $[[REG1:r[0-9]+]], c           {{.*MOVI_i16}}
;CHECK: stb [${{r[0-9]+}}, 0], $[[REG1]]    {{.*STB}}
  %0 = ptrtoint i8* @c to i16
  %1 = trunc i16 %0 to i8
  store i8 %1, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_reg_imm(i8* %x) {
entry:
;CHECK: truncstore_i16_i8_reg_imm:
;CHECK: movi $[[REG1:r[0-9]+]], 215         {{.*MOVI_i16}}
;CHECK: stb [${{r[0-9]+}}, 0], $[[REG1]]    {{.*STB}}
  %0 = trunc i16 4567 to i8
  store i8 %0, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}

define void @truncstore_i16_i8_reg_reg(i8* %x, i16 %y) {
entry:
;CHECK: truncstore_i16_i8_reg_reg:
;CHECK: stb [${{r[0-9]+}}, 0], ${{r[0-9]+}} {{.*STB}}
  %0 = trunc i16 %y to i8
  store i8 %0, i8* %x, align 1
  ret void ;CHECK: jmp    {{.*JMP}}
}
