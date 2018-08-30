; RUN: llc -asm-show-inst -march=aap < %s \
; RUN:   | FileCheck %s -check-prefix=CHECK-FPELIM
; RUN: llc -asm-show-inst -disable-fp-elim -march=aap < %s \
; RUN:   | FileCheck %s -check-prefix=CHECK-WITHFP


; Test that varargs are correctly lowered for callers and callees

declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)

declare void @notdead(i8*)


define i16 @va1(i8* %fmt, ...) nounwind {
; CHECK-FPELIM-LABEL: va1:
; CHECK-FPELIM:  subi $r1, $r1, 2
; CHECK-FPELIM:  addi $[[PTR:r[0-9]+]], $r1, 4
; CHECK-FPELIM:  addi $[[PTR]], $[[PTR]], 2
; CHECK-FPELIM:  stw [$r1, 0], $[[PTR]]
; CHECK-FPELIM:  ldw ${{r[0-9]+}}, [$r1, 4]
; CHECK-FPELIM:  addi $r1, $r1, 2
;
; CHECK-WITHFP-LABEL: va1:
; CHECK-WITHFP:  subi $r1, $r1, 6
; CHECK-WITHFP:  stw [$r1, 4], $r0
; CHECK-WITHFP:  stw [$r1, 2], $r8
; CHECK-WITHFP:  addi $r8, $r1, 6
; CHECK-WITHFP:  addi $[[PTR:r[0-9]+]], $r1, 8
; CHECK-WITHFP:  addi $[[PTR]], $[[PTR]], 2
; CHECK-WITHFP:  stw [$r8, -6], $[[PTR]]
; CHECK-WITHFP:  ldw ${{r[0-9]+}}, [$r1, 8]
; CHECK-WITHFP:  ldw $r8, [$r1, 2]
; CHECK-WITHFP:  ldw $r0, [$r1, 4]
; CHECK-WITHFP:  addi $r1, $r1, 6
  %va = alloca i8*, align 2
  %1 = bitcast i8** %va to i8*
  call void @llvm.va_start(i8* %1)
  %argp.cur = load i8*, i8** %va, align 2
  %argp.next = getelementptr inbounds i8, i8* %argp.cur, i16 2
  store i8* %argp.next, i8** %va, align 2
  %2 = bitcast i8* %argp.cur to i16*
  %3 = load i16, i16* %2, align 2
  call void @llvm.va_end(i8* %1)
  ret i16 %3
}

define i16 @va1_va_arg(i8* %fmt, ...) nounwind {
; CHECK-FPELIM-LABEL: va1_va_arg:
; CHECK-FPELIM:  subi $r1, $r1, 2
; CHECK-FPELIM:  addi $[[PTR:r[0-9]+]], $r1, 4
; CHECK-FPELIM:  addi $[[PTR]], $[[PTR]], 2
; CHECK-FPELIM:  stw [$r1, 0], $[[PTR]]
; CHECK-FPELIM:  ldw ${{r[0-9]+}}, [$r1, 4]
; CHECK-FPELIM:  addi $r1, $r1, 2
;
; CHECK-WITHFP-LABEL: va1_va_arg:
; CHECK-WITHFP:  subi $r1, $r1, 6
; CHECK-WITHFP:  addi $r8, $r1, 6
; CHECK-WITHFP:  addi $[[PTR:r[0-9]+]], $r1, 8
; CHECK-WITHFP:  addi $[[PTR]], $[[PTR]], 2
; CHECK-WITHFP:  stw [$r8, -6], $[[PTR]]
; CHECK-WITHFP:  ldw ${{r[0-9]+}}, [$r1, 8]
; CHECK-WITHFP:  addi $r1, $r1, 6
  %va = alloca i8*, align 2
  %1 = bitcast i8** %va to i8*
  call void @llvm.va_start(i8* %1)
  %2 = va_arg i8** %va, i16
  call void @llvm.va_end(i8* %1)
  ret i16 %2
}

; Ensure the adjustment when restoring the stack pointer using the frame
; pointer is correct
define i16 @va1_va_arg_alloca(i8* %fmt, ...) nounwind {
; CHECK-FPELIM-LABEL: va1_va_arg_alloca:
; CHECK-FPELIM:  subi $r1, $r1, 10
; CHECK-FPELIM:  stw [$r1, 8], $r0
; CHECK-FPELIM:  stw [$r1, 4], $r3
; CHECK-FPELIM:  stw [$r1, 6], $r8
; CHECK-FPELIM:  addi $r8, $r1, 10
; CHECK-FPELIM:  addi $[[PTR:r[0-9]+]], $r1, 12
; CHECK-FPELIM:  addi $[[PTR]], $[[PTR]], 2
; CHECK-FPELIM:  stw [$r1, 0], $[[PTR]]
; CHECK-FPELIM:  ldw $[[ARG1:r[0-9]+]], [$r1, 12]
; CHECK-FPELIM:  addi $[[SPADJ:r[0-9]+]], $[[ARG1]], 1
; CHECK-FPELIM:  movi $[[REG1:r[0-9]+]], -2
; CHECK-FPELIM:  and $[[SPADJ]], $[[SPADJ]], $[[REG1]]
; CHECK-FPELIM:  sub $[[SPADJ]], $r1, $[[SPADJ]]
; CHECK-FPELIM:  mov $r1, $[[SPADJ]]
; CHECK-FPELIM:  bal notdead, $r0
; CHECK-FPELIM:  mov $r2, $[[ARG1]]
; CHECK-FPELIM:  subi $r1, $r8, 10
; CHECK-FPELIM:  ldw $r8, [$r1, 6]
; CHECK-FPELIM:  ldw $r3, [$r1, 4]
; CHECK-FPELIM:  ldw $r0, [$r1, 8]
; CHECK-FPELIM:  addi $r1, $r1, 10
;
; CHECK-WITHFP-LABEL: va1_va_arg_alloca:
; CHECK-WITHFP:  subi $r1, $r1, 10
; CHECK-WITHFP:  stw [$r1, 8], $r0
; CHECK-WITHFP:  stw [$r1, 4], $r3
; CHECK-WITHFP:  stw [$r1, 6], $r8
; CHECK-WITHFP:  addi $r8, $r1, 10
; CHECK-WITHFP:  addi $[[PTR:r[0-9]+]], $r1, 12
; CHECK-WITHFP:  addi $[[PTR]], $[[PTR]], 2
; CHECK-WITHFP:  stw [$r1, 0], $[[PTR]]
; CHECK-WITHFP:  ldw $[[ARG1:r[0-9]+]], [$r1, 12]
; CHECK-WITHFP:  addi $[[SPADJ:r[0-9]+]], $[[ARG1]], 1
; CHECK-WITHFP:  movi $[[REG1:r[0-9]+]], -2
; CHECK-WITHFP:  and $[[SPADJ]], $[[SPADJ]], $[[REG1]]
; CHECK-WITHFP:  sub $[[SPADJ]], $r1, $[[SPADJ]]
; CHECK-WITHFP:  mov $r1, $[[SPADJ]]
; CHECK-WITHFP:  bal notdead, $r0
; CHECK-WITHFP:  mov $r2, $[[ARG1]]
; CHECK-WITHFP:  subi $r1, $r8, 10
; CHECK-WITHFP:  ldw $r8, [$r1, 6]
; CHECK-WITHFP:  ldw $r3, [$r1, 4]
; CHECK-WITHFP:  ldw $r0, [$r1, 8]
; CHECK-WITHFP:  addi $r1, $r1, 10
  %va = alloca i8*, align 2
  %1 = bitcast i8** %va to i8*
  call void @llvm.va_start(i8* %1)
  %2 = va_arg i8** %va, i16
  %3 = alloca i8, i16 %2
  call void @notdead(i8* %3)
  call void @llvm.va_end(i8* %1)
  ret i16 %2
}

define void @va1_caller() nounwind {
; CHECK-FPELIM-LABEL: va1_caller:
; CHECK-FPELIM:  subi $r1, $r1, 10
; CHECK-FPELIM:  stw [$r1, 8], $r0
; CHECK-FPELIM:  stw [$r1, 6], $r2
; CHECK-FPELIM:  movi $[[ARG2:r[0-9]+]], 2
; CHECK-FPELIM:  stw [$r1, 4], $[[ARG2]]
; CHECK-FPELIM:  movi $[[ARG1:r[0-9]+]], 1
; CHECK-FPELIM:  stw [$r1, 2], $[[ARG1]]
; CHECK-FPELIM:  bal va1, $r0
; CHECK-FPELIM:  ldw $r2, [$r1, 6]
; CHECK-FPELIM:  ldw $r0, [$r1, 8]
; CHECK-FPELIM:  addi $r1, $r1, 10
;
; CHECK-WITHFP-LABEL: va1_caller:
; CHECK-WITHFP:  subi $r1, $r1, 12
; CHECK-WITHFP:  stw [$r1, 10], $r0
; CHECK-WITHFP:  stw [$r1, 6], $r2
; CHECK-WITHFP:  stw [$r1, 8], $r8
; CHECK-WITHFP:  addi $r8, $r1, 12
; CHECK-WITHFP:  movi $[[ARG2:r[0-9]+]], 2
; CHECK-WITHFP:  stw [$r1, 4], $[[ARG2]]
; CHECK-WITHFP:  movi $[[ARG1:r[0-9]+]], 1
; CHECK-WITHFP:  stw [$r1, 2], $[[ARG1]]
; CHECK-WITHFP:  bal va1, $r0
; CHECK-WITHFP:  ldw $r8, [$r1, 8]
; CHECK-WITHFP:  ldw $r2, [$r1, 6]
; CHECK-WITHFP:  ldw $r0, [$r1, 10]
; CHECK-WITHFP:  addi $r1, $r1, 12
  %1 = call i16 (i8*, ...) @va1(i8* undef, i16 1, i16 2)
  ret void
}
