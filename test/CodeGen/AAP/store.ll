; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of various simple store operations.


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

; TODO: stb_imm_global


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

; TODO: stb_reg_global


; TODO: stb_global_imm
; TODO: stb_global_reg
; TODO: stb_global_global

; TODO: postinc/predec?


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

; TODO: stw_imm_global


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

; TODO: stw_reg_global


; TODO: stw_global_imm
; TODO: stw_global_reg
; TODO: stw_global_global

; TODO: postinc/predec?
