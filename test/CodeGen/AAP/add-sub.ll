; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Simple tests of basic word size add/sub operations


; ADD

define i16 @add_short_imm(i16 %x) {
entry:
;CHECK: add_short_imm:
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, 3               {{.*ADD_i3_short}}
  %0 = add i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

; TODO: ADD_r is selected instead of ADD_i10
define i16 @add_imm(i16 %x) {
entry:
;CHECK: add_imm:
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = add i16 %x, 123
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @add_big_imm(i16 %x) {
entry:
;CHECK: add_big_imm:
;CHECK: mov ${{r[0-9]+}}, 21345
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = add i16 %x, 21345
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @add_reg(i16 %x, i16 %y) {
entry:
;CHECK: add_reg:
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = add i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; SUB

; TODO: ADD_r is selected instead of SUB_i3_short
define i16 @sub_short_imm(i16 %x) {
entry:
;CHECK: sub_short_imm:
;CHECK: mov ${{r[0-9]+}}, -3
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = sub i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

; TODO: ADD_r is selected instead of SUB_i10
define i16 @sub_imm(i16 %x) {
entry:
;CHECK: sub_imm:
;CHECK: mov ${{r[0-9]+}}, -252
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = sub i16 %x, 252
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

; TODO: ADD_r is selected instead of SUB_r
define i16 @sub_big_imm(i16 %x) {
entry:
;CHECK: sub_big_imm:
;CHECK: mov ${{r[0-9]+}}, -12345
;CHECK: add ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*ADD_r(_short)?}}
  %0 = sub i16 %x, 12345
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @sub_reg(i16 %x, i16 %y) {
entry:
;CHECK: sub_reg:
;CHECK: sub ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*SUB_r(_short)?}}
  %0 = sub i16 %x, %y
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}
