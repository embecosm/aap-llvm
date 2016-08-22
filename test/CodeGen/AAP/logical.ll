; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Simple tests of basic word size logical operations


; AND

define i16 @and_imm(i16 %x) {
entry:
;CHECK: and_imm:
;CHECK: andi ${{r[0-9]+}}, ${{r[0-9]+}}, 3              {{.*ANDI_i9}}
  %0 = and i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @and_big_imm(i16 %x) {
entry:
;CHECK: and_big_imm:
;CHECK: movi ${{r[0-9]+}}, 19283                        {{.*MOVI_i16}}
;CHECK: and ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*AND_r}}
  %0 = and i16 %x, 19283
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; OR

define i16 @or_imm(i16 %x) {
entry:
;CHECK: or_imm:
;CHECK: ori ${{r[0-9]+}}, ${{r[0-9]+}}, 3               {{.*ORI_i9}}
  %0 = or i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @or_big_imm(i16 %x) {
entry:
;CHECK: or_big_imm:
;CHECK: movi ${{r[0-9]+}}, 19283                        {{.*MOVI_i16}}
;CHECK: or ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}     {{.*OR_r}}
  %0 = or i16 %x, 19283
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}


; XOR

define i16 @xor_imm(i16 %x) {
entry:
;CHECK: xor_imm:
;CHECK: xori ${{r[0-9]+}}, ${{r[0-9]+}}, 3              {{.*XORI_i9}}
  %0 = xor i16 %x, 3
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}

define i16 @xor_big_imm(i16 %x) {
entry:
;CHECK: xor_big_imm:
;CHECK: movi ${{r[0-9]+}}, 19283                        {{.*MOVI_i16}}
;CHECK: xor ${{r[0-9]+}}, ${{r[0-9]+}}, ${{r[0-9]+}}    {{.*XOR_r}}
  %0 = xor i16 %x, 19283
  ret i16 %0 ;CHECK: jmp  {{.*JMP}}
}
