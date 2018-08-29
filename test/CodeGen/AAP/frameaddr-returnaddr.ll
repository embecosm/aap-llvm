; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check lowering of frameaddress and returnaddress intrinsics

declare void @notdead(i8*)
declare i8* @llvm.frameaddress(i32)
declare i8* @llvm.returnaddress(i32)

define i8* @test_frameaddress_0() nounwind {
; CHECK-LABEL: test_frameaddress_0:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r8
; CHECK:         addi $r8, $r1, 4
; CHECK:         mov $r2, $r8
; CHECK:         ldw $r8, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = call i8* @llvm.frameaddress(i32 0)
  ret i8* %1 ; CHECK: jmp   {{.*JMP}}
}

define i8* @test_frameaddress_2() nounwind {
; CHECK-LABEL: test_frameaddress_2:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r8
; CHECK:         addi $r8, $r1, 4
; CHECK:         ldw $r2, [$r8, -4]
; CHECK:         ldw $r2, [$r2, -4]
; CHECK:         ldw $r8, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = call i8* @llvm.frameaddress(i32 2)
  ret i8* %1 ; CHECK: jmp   {{.*JMP}}
}

define i8* @test_frameaddress_3_alloca() nounwind {
; CHECK-LABEL: test_frameaddress_3_alloca:
; CHECK:         subi $r1, $r1, 104
; CHECK:         stw [$r1, 102], $r0
; CHECK:         stw [$r1, 100], $r8
; CHECK:         addi $r8, $r1, 104
; CHECK:         subi $r2, $r8, 104
; CHECK:         bal notdead, $r0
; CHECK:         ldw $r2, [$r8, -4]
; CHECK:         ldw $r2, [$r2, -4]
; CHECK:         ldw $r2, [$r2, -4]
; CHECK:         ldw $r8, [$r1, 100]
; CHECK:         ldw $r0, [$r1, 102]
; CHECK:         addi $r1, $r1, 104
  %1 = alloca [100 x i8]
  %2 = bitcast [100 x i8]* %1 to i8*
  call void @notdead(i8* %2)
  %3 = call i8* @llvm.frameaddress(i32 3)
  ret i8* %3 ; CHECK: jmp   {{.*JMP}}
}

define i8* @test_returnaddress_0() nounwind {
; CHECK-LABEL: test_returnaddress_0:
; CHECK:         mov $r2, $r0
  %1 = call i8* @llvm.returnaddress(i32 0)
  ret i8* %1 ; CHECK: jmp   {{.*JMP}}
}

define i8* @test_returnaddress_2() nounwind {
; CHECK-LABEL: test_returnaddress_2:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0
; CHECK:         stw [$r1, 0], $r8
; CHECK:         addi $r8, $r1, 4
; CHECK:         ldw $r2, [$r8, -4]
; CHECK:         ldw $r2, [$r2, -4]
; CHECK:         ldw $r2, [$r2, -2]
; CHECK:         ldw $r8, [$r1, 0]
; CHECK:         ldw $r0, [$r1, 2]
; CHECK:         addi $r1, $r1, 4
  %1 = call i8* @llvm.returnaddress(i32 2)
  ret i8* %1 ; CHECK: jmp   {{.*JMP}}
}
