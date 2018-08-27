; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of stores with offsets


; Word stores to a register with an immediate offset

define void @stw_reg_short_imm_offset_imm(i16* %x) {
entry:
;CHECK: stw_reg_short_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 12345     {{.*MOVI_i16}}
;CHECK-DAG: stw [${{r[0-9]+}}, 2], $[[REG1]]  {{.*STW(_short)?}}
  %0 = ptrtoint i16* %x to i16
  %1 = add i16 %0, 2
  %2 = inttoptr i16 %1 to i16*
  store i16 12345, i16* %2
  ret void ;CHECK: jmp   {{.*JMP}}
}

define void @stw_reg_big_neg_imm_offset_imm(i16* %x) {
entry:
;CHECK: stw_reg_big_neg_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 17291     {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 9280      {{.*MOVI_i16}}
;CHECK-DAG: sub $[[REG3:r[0-9]+]], ${{r[0-9]+}}, $[[REG2]]  {{.*SUB_r(_short)?}}
;CHECK-DAG: stw [$[[REG3]], 0], $[[REG1]]     {{.*STW(_short)?}}
  %0 = ptrtoint i16* %x to i16
  %1 = sub i16 %0, 9280
  %2 = inttoptr i16 %1 to i16*
  store i16 17291, i16* %2
  ret void ;CHECK jmp   {{.*JMP}}
}


; TODO: globals
; TODO: postinc/predec?
