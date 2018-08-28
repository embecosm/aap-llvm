; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of calls to various function types

declare i16 @external_function(i16)

define i16 @test_call_external(i16 %a) nounwind {
; CHECK-LABEL: test_call_external:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal external_function, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = call i16 @external_function(i16 %a)
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @defined_function(i16 %a) nounwind {
; CHECK-LABEL: defined_function:
; CHECK:         addi $r2, $r2, 1
  %1 = add i16 %a, 1
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_call_defined(i16 %a) nounwind {
; CHECK-LABEL: test_call_defined:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         bal defined_function, $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = call i16 @defined_function(i16 %a)
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_call_indirect(i16 (i16)* %a, i16 %b) nounwind {
; CHECK-LABEL: test_call_indirect:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         mov $[[IND1:r[0-9]+]], $r2
; CHECK:         mov $r2, $r3
; CHECK:         jal $[[IND1]], $r0
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  %1 = call i16 %a(i16 %b)
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

; Ensure that calls to fastcc functions aren't rejected. Such calls may be
; introduced when compiling with optimisation.

define fastcc i16 @fastcc_function(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: fastcc_function:
; CHECK:         add $r2, $r2, $r3
 %1 = add i16 %a, %b
 ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_call_fastcc(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: test_call_fastcc:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r4
; CHECK:         mov $r4, $r2
; CHECK:         bal fastcc_function, $r0
; CHECK:         mov $r2, $r4
; CHECK:         ldw $r4, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = call fastcc i16 @fastcc_function(i16 %a, i16 %b)
  ret i16 %a ; CHECK: jmp   {{.*JMP}}
}

declare i16 @external_many_args(i16, i16, i16, i16, i16, i16, i16, i16, i16, i16) nounwind

define i16 @test_call_external_many_args(i16 %a) nounwind {
; CHECK-LABEL: test_call_external_many_args:
; CHECK:         subi $r1, $r1, 20
; CHECK:         stw [$r1, 18], $r0
; CHECK:         stw [$r1, 16], $r3
; CHECK:         stw [$r1, 14], $r4
; CHECK:         stw [$r1, 12], $r5
; CHECK:         stw [$r1, 10], $r6
; CHECK:         stw [$r1, 8], $r7
; CHECK:         mov $r3, $r2
; CHECK:         stw [$r1, 6], $r3
; CHECK:         stw [$r1, 4], $r3
; CHECK:         stw [$r1, 2], $r3
; CHECK:         stw [$r1, 0], $r3
; CHECK:         mov $r4, $r3
; CHECK:         mov $r5, $r3
; CHECK:         mov $r6, $r3
; CHECK:         mov $r7, $r3
; CHECK:         bal external_many_args, $r0
; CHECK:         mov $r2, $r3
; CHECK:         ldw $r7, [$r1, 8]
; CHECK:         ldw $r6, [$r1, 10]
; CHECK:         ldw $r5, [$r1, 12]
; CHECK:         ldw $r4, [$r1, 14]
; CHECK:         ldw $r3, [$r1, 16]
; CHECK:         ldw $r0, [$r1, 18]
; CHECK:         addi $r1, $r1, 20
  %1 = call i16 @external_many_args(i16 %a, i16 %a, i16 %a, i16 %a, i16 %a,
                                    i16 %a, i16 %a, i16 %a, i16 %a, i16 %a)
  ret i16 %a ; CHECK: jmp   {{.*JMP}}
}

define i16 @defined_many_args(i16, i16, i16, i16, i16, i16, i16, i16, i16, i16 %j) nounwind {
; CHECK-LABEL: defined_many_args:
; CHECK:         ldw $[[REG1:r[0-9]+]], [$r1, 6]
; CHECK:         addi $r2, $[[REG1]], 1
  %added = add i16 %j, 1
  ret i16 %added ; CHECK: jmp   {{.*JMP}}
}

define i16 @test_call_defined_many_args(i16 %a) nounwind {
; CHECK-LABEL: test_call_defined_many_args:
; CHECK:         subi $r1, $r1, 20
; CHECK:         stw [$r1, 18], $r0
; CHECK:         stw [$r1, 16], $r3
; CHECK:         stw [$r1, 14], $r4
; CHECK:         stw [$r1, 12], $r5
; CHECK:         stw [$r1, 10], $r6
; CHECK:         stw [$r1, 8], $r7
; CHECK:         stw [$r1, 6], $r2
; CHECK:         stw [$r1, 4], $r2
; CHECK:         stw [$r1, 2], $r2
; CHECK:         stw [$r1, 0], $r2
; CHECK:         mov $r3, $r2
; CHECK:         mov $r4, $r2
; CHECK:         mov $r5, $r2
; CHECK:         mov $r6, $r2
; CHECK:         mov $r7, $r2
; CHECK:         bal defined_many_args, $r0
; CHECK:         ldw $r7, [$r1, 8]
; CHECK:         ldw $r6, [$r1, 10]
; CHECK:         ldw $r5, [$r1, 12]
; CHECK:         ldw $r4, [$r1, 14]
; CHECK:         ldw $r3, [$r1, 16]
; CHECK:         ldw $r0, [$r1, 18]
; CHECK:         addi $r1, $r1, 20
  %1 = call i16 @defined_many_args(i16 %a, i16 %a, i16 %a, i16 %a, i16 %a,
                                   i16 %a, i16 %a, i16 %a, i16 %a, i16 %a)
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}
