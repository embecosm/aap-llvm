; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Simple tests of basic word size shift operations


; ASR

define i16 @asr_small_imm(i16 %x) {
entry:
;CHECK: asr_small_imm:
;CHECK: asr ${{r[0-9]+}}, ${{r[0-9]+}}, 3               {{.*ASR_i3_short}}
  %0 = ashr i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @asr_imm(i16 %x) {
entry:
;CHECK: asr_imm:
;CHECK: asr ${{r[0-9]+}}, ${{r[0-9]+}}, 14              {{.*ASR_i6}}
  %0 = ashr i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @asr_reg(i16 %x, i16 %y) {
entry:
;CHECK: asr_reg:
;CHECK: asr ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ASR_r(_short)?}}
  %0 = ashr i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; LSL

define i16 @lsl_small_imm(i16 %x) {
entry:
;CHECK: lsl_small_imm:
;CHECK: lsl ${{r[0-9]+}}, ${{r[0-9]+}}, 3               {{.*LSL_i3_short}}
  %0 = shl i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsl_imm(i16 %x) {
entry:
;CHECK: lsl_imm:
;CHECK: lsl ${{r[0-9]+}}, ${{r[0-9]+}}, 14              {{.*LSL_i6}}
  %0 = shl i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsl_reg(i16 %x, i16 %y) {
entry:
;CHECK: lsl_reg:
;CHECK: lsl ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*LSL_r(_short)?}}
  %0 = shl i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; LSR

define i16 @lsr_small_imm(i16 %x) {
entry:
;CHECK: lsr_small_imm:
;CHECK: lsr ${{r[0-9]+}}, ${{r[0-9]+}}, 3               {{.*LSR_i3_short}}
  %0 = lshr i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsr_imm(i16 %x) {
entry:
;CHECK: lsr_imm:
;CHECK: lsr ${{r[0-9]+}}, ${{r[0-9]+}}, 14              {{.*LSR_i6}}
  %0 = lshr i16 %x, 14
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @lsr_reg(i16 %x, i16 %y) {
entry:
;CHECK: lsr_reg:
;CHECK: lsr ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*LSR_r(_short)?}}
  %0 = lshr i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}
