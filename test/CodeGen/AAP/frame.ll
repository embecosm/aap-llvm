; RUN: llc -asm-show-inst -march=aap < %s \
; RUN:   | FileCheck %s -check-prefix=CHECK-FPELIM
; RUN: llc -asm-show-inst -disable-fp-elim -march=aap < %s \
; RUN:   | FileCheck %s -check-prefix=CHECK-WITHFP


; Check that frame index elimination behaves correctly, including in
; the case where the offset is > 10 bits.

%struct.key_t = type [502 x i8]

define i16 @test() nounwind {
; CHECK-FPELIM-LABEL: test:
; CHECK-FPELIM:   subi $r1, $r1, 512
; CHECK-FPELIM:   stw [$r1, 510], $r0
; CHECK-FPELIM:   stw [$r1, 508], $r3
; CHECK-FPELIM:   stw [$r1, 506], $r4
; CHECK-FPELIM:   stw [$r1, 504], $r5
; CHECK-FPELIM:   addi $r5, $r1, 2
; CHECK-FPELIM:   mov $r2, $r5
; CHECK-FPELIM:   movi $r3, 0
; CHECK-FPELIM:   movi $r4, 512
; CHECK-FPELIM:   bal memset, $r0
; CHECK-FPELIM:   mov $r2, $r5
; CHECK-FPELIM:   bal test1, $r0
; CHECK-FPELIM:   movi $r2, 0
; CHECK-FPELIM:   ldw $r5, [$r1, 504]
; CHECK-FPELIM:   ldw $r4, [$r1, 506]
; CHECK-FPELIM:   ldw $r3, [$r1, 508]
; CHECK-FPELIM:   ldw $r0, [$r1, 510]
; CHECK-FPELIM:   addi $r1, $r1, 512
;
; CHECK-WITHFP-LABEL: test:
; CHECK-WITHFP:  subi $r1, $r1, 514
; CHECK-WITHFP:  movi $[[REG1:r[0-9]+]], 512
; CHECK-WITHFP:  add $[[REG1]], $r1, $r2
; CHECK-WITHFP:  stw [$[[REG1]], 0], $r0
; CHECK-WITHFP:  stw [$r1, 508], $r3
; CHECK-WITHFP:  stw [$r1, 506], $r4
; CHECK-WITHFP:  stw [$r1, 504], $r5
; CHECK-WITHFP:  stw [$r1, 510], $r8
; CHECK-WITHFP:  addi $r8, $r1, 514
; CHECK-WITHFP:  addi $r5, $r1, 0
; CHECK-WITHFP:  mov $r2, $r5
; CHECK-WITHFP:  movi $r3, 0
; CHECK-WITHFP:  movi $r4, 512
; CHECK-WITHFP:  bal memset, $r0
; CHECK-WITHFP:  mov $r2, $r5
; CHECK-WITHFP:  bal test1, $r0
; CHECK-WITHFP:  movi $r2, 0
; CHECK-WITHFP:  ldw $r8, [$r1, 510]
; CHECK-WITHFP:  ldw $r5, [$r1, 504]
; CHECK-WITHFP:  ldw $r4, [$r1, 506]
; CHECK-WITHFP:  ldw $r3, [$r1, 508]
; CHECK-WITHFP:  movi $[[REG2:r[0-9]+]], 512
; CHECK-WITHFP:  add $[[REG2]], $r1, $[[REG2]]
; CHECK-WITHFP:  ldw $r0, [$[[REG2]], 0]
; CHECK-WITHFP:  addi $r1, $r1, 514
  %key = alloca %struct.key_t, align 2
  %1 = bitcast %struct.key_t* %key to i8*
  call void @llvm.memset.p0i8.i64(i8* align 2 %1, i8 0, i64 512, i1 false)
  call void @test1(i8* %1)
  ret i16 0 ; CHECK: jmp   {{.*JMP}}
}

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i1)

declare void @test1(i8*)
