; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of stores with offsets


@i8_array = external global [12345 x i8]
@i16_array = external global [12345 x i16]

@i8_ptr = external global i8
@i16_ptr = external global i16


; Byte stores with positive immediate offsets


define void @stb_global_zero_imm_offset_imm() {
entry:
;CHECK: stb_global_zero_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], i8_array  {{.*MOVI_i16}}
;CHECK-DAG: stb [$[[REG2]], 0], $[[REG1]]     {{.*STB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 0
  store i8 123, i8* %0
  ret void ;CHECK jmp   {{.*JMP}}
}

define void @stb_global_short_imm_offset_imm() {
entry:
;CHECK: stb_global_short_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], i8_array  {{.*MOVI_i16}}
;CHECK-DAG: stb [$[[REG2]], 3], $[[REG1]]     {{.*STB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 3
  store i8 123, i8* %0
  ret void ;CHECK jmp   {{.*JMP}}
}

define void @stb_global_imm_offset_imm() {
entry:
;CHECK: stb_global_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], i8_array  {{.*MOVI_i16}}
;CHECK-DAG: stb [$[[REG2]], 4], $[[REG1]]     {{.*STB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 4
  store i8 123, i8* %0
  ret void ;CHECK jmp   {{.*JMP}}
}


; Byte stores with negative immediate offsets


define void @stb_global_neg_imm_offset_imm() {
entry:
;CHECK: stb_global_neg_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 123       {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], i8_array  {{.*MOVI_i16}}
;CHECK-DAG: stb [$[[REG2]], 511], $[[REG1]]   {{.*STB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 511
  store i8 123, i8* %0
  ret void ;CHECK jmp   {{.*JMP}}
}

define void @stb_global_big_neg_imm_offset_imm() {
entry:
;CHECK: stb_global_big_neg_imm_offset_imm:
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 123      {{.*MOVI_i16}}
;CHECK-DAG: movi ${{r[0-9]+}}, i8_array      {{.*MOVI_i16}}
;CHECK-DAG: subi $[[REG2:r[0-9]+]], ${{r[0-9]+}}, 513   {{.*SUBI_i10}}
;CHECK-DAG: stb [$[[REG2]], 0], $[[REG1]]   {{.*STB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -513
  store i8 123, i8* %0
  ret void ;CHECK jmp   {{.*JMP}}
}


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
;CHECK-DAG: movi $[[REG1:r[0-9]+]], 17291    {{.*MOVI_i16}}
;CHECK-DAG: movi $[[REG2:r[0-9]+]], 9280     {{.*MOVI_i16}}
;CHECK-DAG: sub $[[REG3:r[0-9]+]], ${{r[0-9]+}}, $[[REG2]]  {{.*SUB_r(_short)?}}
;CHECK-DAG: stw [$[[REG3]], 0], $[[REG1]]   {{.*STW(_short)?}}
  %0 = ptrtoint i16* %x to i16
  %1 = sub i16 %0, 9280
  %2 = inttoptr i16 %1 to i16*
  store i16 17291, i16* %2
  ret void ;CHECK jmp   {{.*JMP}}
}
