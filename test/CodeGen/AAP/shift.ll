; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Simple tests of basic word size shift operations


; ASR

define i16 @asr_small_imm(i16 %x) {
entry:
;CHECK: asr_small_imm:
;CHECK: asri ${{r[0-9]+}}, ${{r[0-9]+}}, 3              {{.*ASRI_i3_short}}
  %0 = ashr i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @asr_imm(i16 %x) {
entry:
;CHECK: asr_imm:
;CHECK: asri ${{r[0-9]+}}, ${{r[0-9]+}}, 14             {{.*ASRI_i6}}
  %0 = ashr i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @asr_reg(i16 %x, i16 %y) {
entry:
;CHECK: asr_reg:
;CHECK: asr ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ASR_r}}
  %0 = ashr i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; LSL

define i16 @lsl_small_imm(i16 %x) {
entry:
;CHECK: lsl_small_imm:
;CHECK: lsli ${{r[0-9]+}}, ${{r[0-9]+}}, 3              {{.*LSLI_i3_short}}
  %0 = shl i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsl_imm(i16 %x) {
entry:
;CHECK: lsl_imm:
;CHECK: lsli ${{r[0-9]+}}, ${{r[0-9]+}}, 14             {{.*LSLI_i6}}
  %0 = shl i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsl_reg(i16 %x, i16 %y) {
entry:
;CHECK: lsl_reg:
;CHECK: lsl ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*LSL_r}}
  %0 = shl i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; LSR

define i16 @lsr_small_imm(i16 %x) {
entry:
;CHECK: lsr_small_imm:
;CHECK: lsri ${{r[0-9]+}}, ${{r[0-9]+}}, 3              {{.*LSRI_i3_short}}
  %0 = lshr i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsr_imm(i16 %x) {
entry:
;CHECK: lsr_imm:
;CHECK: lsri ${{r[0-9]+}}, ${{r[0-9]+}}, 14             {{.*LSRI_i6}}
  %0 = lshr i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsr_reg(i16 %x, i16 %y) {
entry:
;CHECK: lsr_reg:
;CHECK: lsr ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*LSR_r}}
  %0 = lshr i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}
