; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of truncstore operations in codegen.


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


; TODO: globals
; TODO: postinc/predec?
