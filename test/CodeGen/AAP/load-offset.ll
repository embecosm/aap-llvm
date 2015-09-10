; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of loads with offsets


@a = external global [11 x i16]
@b = external global [8672 x i16]

@i8_array = external global [12345 x i8]
@i16_array = external global [12345 x i16]

@i8_ptr = external global i8
@i16_ptr = external global i16



; Byte loads with positive immediate offsets


define i8 @ldb_global_zero_imm() {
entry:
;CHECK: ldb_global_zero_imm:
;CHECK: mov $[[REG1:r[0-9]+]], i8_array       {{.*MOV_i16}}
;CHECK: ldb ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 0
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_global_short_imm_1() {
entry:
;CHECK: ldb_global_short_imm_1:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array   {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 2]  {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 2
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is only just small enough to fit in the LDB_short offset.
define i8 @ldb_global_short_imm_2() {
entry:
;CHECK: ldb_global_short_imm_2:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array   {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 3]  {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 3
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is just too large for the LDB_short offset
define i8 @ldb_global_short_imm_3() {
entry:
;CHECK: ldb_global_short_imm_3:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array   {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 4]  {{.*LDB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 4
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDB_short offset field, but will still
; fit in the LDB offset.
define i8 @ldb_global_imm_1() {
entry:
;CHECK: ldb_global_imm_1:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array     {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 26]   {{.*LDB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 26
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is large enough to exceed the signed LDB offset field,
; but small enough to fit in the ADD_i10 unsigned immediate field.
define i8 @ldb_global_imm_2() {
entry:
;CHECK: ldb_global_imm_2:
;CHECK: mov ${{r[0-9]+}}, i8_array                    {{.*MOV_i16}}
;CHECK-DAG: add $[[REG2:r[0-9]+]], ${{r[0-9]+}}, 768  {{.*ADD_i10}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG2]], 0]          {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 768
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDB offset field
define i8 @ldb_global_big_imm() {
entry:
;CHECK: ldb_global_big_imm:
;CHECK-DAG: mov ${{r[0-9]+}}, i8_array        {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, 1234            {{.*MOV_i16}}
;CHECK-NOT: BARRIER
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}} {{.*ADD_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                  {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 1234
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}


; Byte load with negative immediate offsets


define i8 @ldb_global_short_neg_imm_1() {
entry:
;CHECK: ldb_global_short_neg_imm_1:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array     {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], -2]   {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -2
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is only just small enough to fit in the LDB_short offset.
define i8 @ldb_global_short_neg_imm_2() {
entry:
;CHECK: ldb_global_short_neg_imm_2:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array     {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], -4]   {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -4
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is just too large for the LDB_short offset
define i8 @ldb_global_short_neg_imm_3() {
entry:
;CHECK: ldb_global_short_neg_imm_3:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array     {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], -5]   {{.*LDB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -5
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDB_short offset field, but will still
; fit in the LDB offset.
define i8 @ldb_global_neg_imm_1() {
entry:
;CHECK: ldb_global_neg_imm_1:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i8_array     {{.*MOV_i16}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], -26]  {{.*LDB$}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -26
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is large enough to exceed the signed LDB offset field,
; but small enough to fit in the SUB_i10 unsigned immediate field.
define i8 @ldb_global_neg_imm_2() {
entry:
;CHECK: ldb_global_neg_imm_2:
;CHECK: mov ${{r[0-9]+}}, i8_array                    {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG2:r[0-9]+]], ${{r[0-9]+}}, 755  {{.*SUB_i10}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG2]], 0]          {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -755
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDB offset field
define i8 @ldb_global_big_neg_imm() {
entry:
;CHECK: ldb_global_big_neg_imm:
;CHECK-DAG: mov ${{r[0-9]+}}, i8_array  {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, 1234      {{.*MOV_i16}}
;CHECK-NOT: BARRIER
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 -1234
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}


; Byte loads from a register with an immediate offset


define i8 @ldb_reg_imm(i8* %x) {
entry:
;CHECK: ldb_reg_imm:
;CHECK-DAG: ldb ${{r[0-9]+}}, [${{r[0-9]+}}, 15]  {{.*LDB(_short)?}}
  %0 = ptrtoint i8* %x to i16
  %1 = add i16 %0, 15
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_reg_big_imm(i8* %x) {
entry:
;CHECK: ldb_reg_big_imm:
;CHECK mov ${{r[0-9]+}}, 12345                {{.*MOV_i16}}
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*ADD_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = ptrtoint i8* %x to i16
  %1 = add i16 %0, 12345
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}

; With a negative offset
define i8 @ldb_reg_neg_imm(i8* %x) {
entry:
;CHECK: ldb_reg_neg_imm:
;CHECK: ldb ${{r[0-9]+}}, [${{r[0-9]+}}, -432]  {{.*LDB$}}
  %0 = ptrtoint i8* %x to i16
  %1 = sub i16 %0, 432
  %2 = inttoptr i16 %1 to i8*
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}


; Byte loads from a global with a register offset


define i8 @ldb_global_reg(i16 %x) {
entry:
;CHECK: ldb_global_reg:
;CHECK-DAG: mov ${{r[0-9]+}}, i8_array        {{.*MOV_i16}}
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*ADD_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 %x
  %1 = load i8, i8* %0
  ret i8 %1 ;CHECK: jmp   {{.*JMP}}
}

; With a negative register offset
define i8 @ldb_global_neg_reg(i16 %x) {
entry:
;CHECK: ldb_global_neg_reg:
;CHECK: mov ${{r[0-9]+}}, i8_array            {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = sub i16 0, %x
  %1 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 %0
  %2 = load i8, i8* %1
  ret i8 %2 ;CHECK: jmp   {{.*JMP}}
}


; Byte loads from a global with a global offset
define i8 @ldb_global_global() {
entry:
;CHECK: ldb_global_global:
;CHECK-DAG: mov ${{r[0-9]+}}, i8_array        {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, i8_ptr          {{.*MOV_i16}}
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*ADD_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = ptrtoint i8* @i8_ptr to i16
  %1 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 %0
  %2 = load i8, i8* %1
  ret i8 %2 ;CHECK: jmp   {{.*JMP}}
}

define i8 @ldb_global_neg_global() {
entry:
;CHECK: ldb_global_neg_global:
;CHECK-DAG: mov ${{r[0-9]+}}, i8_array        {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, i8_ptr          {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r(_short)?}}
;CHECK-DAG: ldb ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDB(_short)?}}
  %0 = ptrtoint i8* @i8_ptr to i16
  %1 = sub i16 0, %0
  %2 = getelementptr [12345 x i8], [12345 x i8]* @i8_array, i16 0, i16 %1
  %3 = load i8, i8* %2
  ret i8 %3 ;CHECK: jmp   {{.*JMP}}
}


; Word loads with positive immediate offsets


define i16 @ldw_global_zero_imm() {
entry:
;CHECK: ldw_global_zero_imm:
;CHECK: mov $[[REG1:r[0-9]+]], i16_array      {{.*MOV_i16}}
;CHECK: ldw ${{r[0-9]+}}, [$[[REG1]], 0]      {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 0
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}

define i16 @ldw_global_short_imm() {
entry:
;CHECK: ldw_global_short_imm:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i16_array  {{.*MOV_i16}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 2]  {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 1
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDW_short offset field, but will still
; fit in the LDW offset.
define i16 @ldw_global_imm() {
entry:
;CHECK: ldw_global_imm:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i16_array    {{.*MOV_i16}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 424]  {{.*LDW$}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 212
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is too large for the LDB offset field
define i16 @ldw_global_big_imm() {
entry:
;CHECK: ldw_global_big_imm:
;CHECK-DAG: mov ${{r[0-9]+}}, i16_array       {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, 2468            {{.*MOV_i16}}
;CHECK-NOT: BARRIER
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}} {{.*ADD_r(_short)?}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 0]                  {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 1234
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}


; Word load with negative immediate offsets


define i16 @ldw_global_short_neg_imm() {
entry:
;CHECK: ldw_global_short_neg_imm:
;CHECK-DAG: mov $[[REG1:r[0-9]+]], i16_array    {{.*MOV_i16}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], -4]   {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 -2
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}

; The immediate is large enough to exceed the signed LDW offset field,
; but small enough to fit in the SUB_i10 unsigned immediate field.
define i16 @ldw_global_neg_imm() {
entry:
;CHECK: ldw_global_neg_imm:
;CHECK: mov ${{r[0-9]+}}, i16_array                   {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG2:r[0-9]+]], ${{r[0-9]+}}, 634  {{.*SUB_i10}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG2]], 0]          {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 -317
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}


; Word loads from a register with an immediate offset


define i16 @ldw_reg_imm(i16* %x) {
entry:
;CHECK: ldw_reg_imm:
;CHECK-DAG: ldw ${{r[0-9]+}}, [${{r[0-9]+}}, 16]  {{.*LDW(_short)?}}
  %0 = ptrtoint i16* %x to i16
  %1 = add i16 %0, 16
  %2 = inttoptr i16 %1 to i16*
  %3 = load i16, i16* %2
  ret i16 %3 ;CHECK: jmp   {{.*JMP}}
}

define i16 @ldw_reg_big_neg_imm(i16* %x) {
entry:
;CHECK: ldw_reg_big_neg_imm:
;CHECK mov ${{r[0-9]+}}, 22222                {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r(_short)?}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDW(_short)?}}
  %0 = ptrtoint i16* %x to i16
  %1 = sub i16 %0, 22222
  %2 = inttoptr i16 %1 to i16*
  %3 = load i16, i16* %2
  ret i16 %3 ;CHECK: jmp   {{.*JMP}}
}


; Byte loads from a global with a register offset


define i16 @ldw_global_reg(i16 %x) {
entry:
;CHECK: ldw_global_reg:
;CHECK-DAG: mov ${{r[0-9]+}}, i16_array       {{.*MOV_i16}}
;CHECK-DAG: add $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*ADD_r(_short)?}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDW(_short)?}}
  %0 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 %x
  %1 = load i16, i16* %0
  ret i16 %1 ;CHECK: jmp   {{.*JMP}}
}


; Byte loads from a global with a global offset


define i16 @ldw_global_neg_global() {
entry:
;CHECK: ldw_global_neg_global:
;CHECK-DAG: mov ${{r[0-9]+}}, i16_array       {{.*MOV_i16}}
;CHECK-DAG: mov ${{r[0-9]+}}, i16_ptr         {{.*MOV_i16}}
;CHECK-DAG: sub $[[REG1:r[0-9]+]], ${{r[0-9]+}}, ${{r[0-9]+}}   {{.*SUB_r(_short)?}}
;CHECK-DAG: ldw ${{r[0-9]+}}, [$[[REG1]], 0]                    {{.*LDW(_short)?}}
  %0 = ptrtoint i16* @i16_ptr to i16
  %1 = sub i16 0, %0
  %2 = getelementptr [12345 x i16], [12345 x i16]* @i16_array, i16 0, i16 %1
  %3 = load i16, i16* %2
  ret i16 %3 ;CHECK: jmp   {{.*JMP}}
}
