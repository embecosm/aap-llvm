; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of expanded bit operations

declare i16 @llvm.bswap.i16(i16)
declare i32 @llvm.bswap.i32(i32)
declare i8 @llvm.cttz.i8(i8, i1)
declare i16 @llvm.cttz.i16(i16, i1)
declare i32 @llvm.cttz.i32(i32, i1)
declare i16 @llvm.ctlz.i16(i16, i1)
declare i32 @llvm.ctlz.i32(i32, i1)
declare i16 @llvm.ctpop.i16(i16)

define i16 @test_bswap_i16(i16 %a) nounwind {
; CHECK-LABEL: test_bswap_i16:
; CHECK:         lsli $[[REG1:r[0-9]+]], $r2, 8
; CHECK:         lsri $r2, $r2, 8
; CHECK:         or $r2, $r2, $[[REG1]]
  %tmp = call i16 @llvm.bswap.i16(i16 %a)
  ret i16 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i32 @test_bswap_i32(i32 %a) nounwind {
; CHECK-LABEL: test_bswap_i32:
; CHECK:         lsli $[[REG1:r[0-9]+]], $r3, 8
; CHECK:         lsri $[[REG2:r[0-9]+]], $r3, 8
; CHECK:         or $[[REG1]], $[[REG2]], $[[REG1]]
; CHECK:         lsli $[[REG2]], $r2, 8
; CHECK:         lsri $r2, $r2, 8
; CHECK:         or $r3, $r2, $[[REG2]]
; CHECK:         mov $r2, $[[REG1]]
  %tmp = call i32 @llvm.bswap.i32(i32 %a)
  ret i32 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_cttz_i16(i16 %a) nounwind {
; CHECK-LABEL: test_cttz_i16:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         movi $[[REG1:r[0-9]+]], 0
; CHECK:         beq .[[PRE:LBB[0-9]+_[0-9]+]], $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], -1
; CHECK:         xor $[[REG1]], $r2, $[[REG1]]
; CHECK:         subi $r2, $r2, 1
; CHECK:         and $r2, $[[REG1]], $r2
; CHECK:         lsri $[[REG1]], $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], 21845
; CHECK:         and $[[REG1]], $[[REG1]], $[[REG2]]
; CHECK:         sub $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 13107
; CHECK:         and $[[REG2]], $r2, $[[REG1]]
; CHECK:         lsri $r2, $r2, 2
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         add $r2, $[[REG2]], $r2
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 3855
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         movi $r3, 257
; CHECK:         bal __mulhi3, $r0
; CHECK:         lsri $r2, $r2, 8
; CHECK:         bra .[[END:LBB[0-9]+_[0-9]+]]
; CHECK:       .[[PRE]]
; CHECK:         movi $r2, 16
; CHECK:       .[[END]]:
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %tmp = call i16 @llvm.cttz.i16(i16 %a, i1 false)
  ret i16 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_ctlz_i16(i16 %a) nounwind {
; CHECK-LABEL: test_ctlz_i16:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         movi $[[REG1:r[0-9]+]], 0
; CHECK:         beq .[[PRE:LBB[0-9]+_[0-9]+]], $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 1
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 2
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 8
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], -1
; CHECK:         xor $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 21845
; CHECK:         lsri $[[REG2:r[0-9]+]], $r2, 1
; CHECK:         and $[[REG1]], $[[REG2]], $[[REG1]]
; CHECK:         sub $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 13107
; CHECK:         and $[[REG2]], $r2, $[[REG1]]
; CHECK:         lsri $r2, $r2, 2
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         add $r2, $[[REG2]], $r2
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 3855
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         movi $r3, 257
; CHECK:         bal __mulhi3, $r0
; CHECK:         lsri $r2, $r2, 8
; CHECK:         bra .[[END:LBB[0-9]+_[0-9]+]]
; CHECK:       .[[PRE]]:
; CHECK:         movi $r2, 16
; CHECK:       .[[END]]:
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %tmp = call i16 @llvm.ctlz.i16(i16 %a, i1 false)
  ret i16 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_cttz_i16_zero_undef(i16 %a) nounwind {
; CHECK-LABEL: test_cttz_i16_zero_undef:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         movi $[[REG1:r[0-9]+]], -1
; CHECK:         xor $[[REG1]], $r2, $[[REG1]]
; CHECK:         subi $r2, $r2, 1
; CHECK:         and $r2, $[[REG1]], $r2
; CHECK:         lsri $[[REG1]], $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], 21845
; CHECK:         and $[[REG1]], $[[REG1]], $[[REG2]]
; CHECK:         sub $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 13107
; CHECK:         and $[[REG2]], $r2, $[[REG1]]
; CHECK:         lsri $r2, $r2, 2
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         add $r2, $[[REG2]], $r2
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 3855
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         movi $r3, 257
; CHECK:         bal __mulhi3, $r0
; CHECK:         lsri $r2, $r2, 8
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %tmp = call i16 @llvm.cttz.i16(i16 %a, i1 true)
  ret i16 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_ctlz_i16_zero_undef(i16 %a) nounwind {
; CHECK-LABEL: test_ctlz_i16_zero_undef:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         lsri $[[REG1:r[0-9]+]], $r2, 1
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 2
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         lsri $[[REG1]], $r2, 8
; CHECK:         or $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], -1
; CHECK:         xor $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 21845
; CHECK:         lsri $[[REG2:r[0-9]+]], $r2, 1
; CHECK:         and $[[REG1]], $[[REG2:r[0-9]+]], $[[REG1]]
; CHECK:         sub $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 13107
; CHECK:         and $[[REG2:r[0-9]+]], $r2, $[[REG1]]
; CHECK:         lsri $r2, $r2, 2
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         add $r2, $[[REG2:r[0-9]+]], $r2
; CHECK:         lsri $[[REG1]], $r2, 4
; CHECK:         add $r2, $r2, $[[REG1]]
; CHECK:         movi $[[REG1]], 3855
; CHECK:         and $r2, $r2, $[[REG1]]
; CHECK:         movi $r3, 257
; CHECK:         bal __mulhi3, $r0
; CHECK:         lsri $r2, $r2, 8
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %tmp = call i16 @llvm.ctlz.i16(i16 %a, i1 true)
  ret i16 %tmp ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_ctpop_i16(i16 %a) nounwind {
; CHECK-LABEL: test_ctpop_i16:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r3
; CHECK:         lsri $r10, $r2, 1
; CHECK:         movi $r13, 21845
; CHECK:         and $r10, $r10, $r13
; CHECK:         sub $r2, $r2, $r10
; CHECK:         movi $r10, 13107
; CHECK:         and $r13, $r2, $r10
; CHECK:         lsri $r2, $r2, 2
; CHECK:         and $r2, $r2, $r10
; CHECK:         add $r2, $r13, $r2
; CHECK:         lsri $r10, $r2, 4
; CHECK:         add $r2, $r2, $r10
; CHECK:         movi $r10, 3855
; CHECK:         and $r2, $r2, $r10
; CHECK:         movi $r3, 257
; CHECK:         bal __mulhi3, $r0
; CHECK:         lsri $r2, $r2, 8
; CHECK:         ldw $r3, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = call i16 @llvm.ctpop.i16(i16 %a)
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}
