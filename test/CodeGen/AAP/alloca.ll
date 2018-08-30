; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check that alloca, stacksave and stackrestore can be expanded
; correctly, and the frame pointer is used to restore the stack pointer
; as expected.

declare void @notdead(i8*)

; These tests must ensure the stack pointer is restored using the frame
; pointer

define void @simple_alloca(i16 %n) nounwind {
; CHECK-LABEL: simple_alloca:
; CHECK:         subi $r1, $r1, 8
; CHECK:         stw [$r1, 6], $r0
; CHECK:         stw [$r1, 2], $r2
; CHECK:         stw [$r1, 4], $r8
; CHECK:         addi $r8, $r1, 8
; CHECK:         addi $[[REG1:r[0-9]+]], $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], -2
; CHECK:         and $[[REG1]], $[[REG1]], $[[REG2]]
; CHECK:         sub $r2, $r1, $[[REG1]]
; CHECK:         mov $r1, $r2
; CHECK:         bal notdead, $r0
; CHECK:         subi $r1, $r8, 8
; CHECK:         ldw $r8, [$r1, 4]
; CHECK:         ldw $r2, [$r1, 2]
; CHECK:         ldw $r0, [$r1, 6]
; CHECK:         addi $r1, $r1, 8
  %1 = alloca i8, i16 %n
  call void @notdead(i8* %1)
  ret void ; CHECK: jmp   {{.*JMP}}
}

declare i8* @llvm.stacksave()
declare void @llvm.stackrestore(i8*)

define void @scoped_alloca(i16 %n) nounwind {
; CHECK-LABEL: scoped_alloca:
; CHECK:         subi $r1, $r1, 10
; CHECK:         stw [$r1, 8], $r0
; CHECK:         stw [$r1, 4], $r2
; CHECK:         stw [$r1, 2], $r3
; CHECK:         stw [$r1, 6], $r8
; CHECK:         addi $r8, $r1, 10
; CHECK:         addi $[[REG1:r[0-9]+]], $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], -2
; CHECK:         and $[[REG1]], $[[REG1]], $[[REG2]]
; CHECK:         mov $r3, $r1
; CHECK:         sub $r2, $r1, $[[REG1]]
; CHECK:         mov $r1, $r2
; CHECK:         bal notdead, $r0
; CHECK:         mov $r1, $r3
; CHECK:         subi $r1, $r8, 10
; CHECK:         ldw $r8, [$r1, 6]
; CHECK:         ldw $r3, [$r1, 2]
; CHECK:         ldw $r2, [$r1, 4]
; CHECK:         ldw $r0, [$r1, 8]
; CHECK:         addi $r1, $r1, 10
  %sp = call i8* @llvm.stacksave()
  %addr = alloca i8, i16 %n
  call void @notdead(i8* %addr)
  call void @llvm.stackrestore(i8* %sp)
  ret void ; CHECK: jmp   {{.*JMP}}
}

declare void @func(i8*, i16, i16, i16, i16, i16, i16, i16, i16, i16, i16, i16)

; Check that outgoing arguments passed on the stack do not corrupt a
; variable-sized stack object.
define void @alloca_callframe(i16 %n) nounwind {
; CHECK-LABEL: alloca_callframe:
; CHECK:         subi $r1, $r1, 18
; CHECK:         stw [$r1, 16], $r0
; CHECK:         stw [$r1, 12], $r2
; CHECK:         stw [$r1, 10], $r3
; CHECK:         stw [$r1, 8], $r4
; CHECK:         stw [$r1, 6], $r5
; CHECK:         stw [$r1, 4], $r6
; CHECK:         stw [$r1, 2], $r7
; CHECK:         stw [$r1, 14], $r8
; CHECK:         addi $r8, $r1, 18
; CHECK:         addi $[[REG1:r[0-9]+]], $r2, 1
; CHECK:         movi $[[REG2:r[0-9]+]], -2
; CHECK:         and $[[REG1]], $[[REG1]], $[[REG2]]
; CHECK:         sub $r2, $r1, $[[REG1]]
; CHECK:         mov $r1, $r2
; CHECK:         subi $r1, $r1, 12
; CHECK:         movi $[[REG1]], 12
; CHECK:         stw [$r1, 10], $[[REG1]]
; CHECK:         movi $[[REG1]], 11
; CHECK:         stw [$r1, 8], $[[REG1]]
; CHECK:         movi $[[REG1]], 10
; CHECK:         stw [$r1, 6], $[[REG1]]
; CHECK:         movi $[[REG1]], 9
; CHECK:         stw [$r1, 4], $[[REG1]]
; CHECK:         movi $[[REG1]], 8
; CHECK:         stw [$r1, 2], $[[REG1]]
; CHECK:         movi $[[REG1]], 7
; CHECK:         stw [$r1, 0], $[[REG1]]
; CHECK:         movi $r3, 2
; CHECK:         movi $r4, 3
; CHECK:         movi $r5, 4
; CHECK:         movi $r6, 5
; CHECK:         movi $r7, 6
; CHECK:         bal func, $r0
; CHECK:         addi $r1, $r1, 12
; CHECK:         subi $r1, $r8, 18
; CHECK:         ldw $r8, [$r1, 14]
; CHECK:         ldw $r7, [$r1, 2]
; CHECK:         ldw $r6, [$r1, 4]
; CHECK:         ldw $r5, [$r1, 6]
; CHECK:         ldw $r4, [$r1, 8]
; CHECK:         ldw $r3, [$r1, 10]
; CHECK:         ldw $r2, [$r1, 12]
; CHECK:         ldw $r0, [$r1, 16]
; CHECK:         addi $r1, $r1, 18
  %1 = alloca i8, i16 %n
  call void @func(i8* %1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 8,
                  i16 9, i16 10, i16 11, i16 12)
  ret void ; CHECK: jmp   {{.*JMP}}
}
