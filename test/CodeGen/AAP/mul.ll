; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check correctness of various expanded multiplications

define i16 @square(i16 %a) nounwind {
; CHECK-LABEL: square:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0           ; 2-byte Folded Spill
; CHECK:         stw [$r1, 0], $r3           ; 2-byte Folded Spill
; CHECK:         mov $r3, $r2
; CHECK:         bal __mulhi3, $r0
; CHECK:         ldw $r3, [$r1, 0]           ; 2-byte Folded Reload
; CHECK:         ldw $r0, [$r1, 2]           ; 2-byte Folded Reload
; CHECK:         addi $r1, $r1, 4
  %1 = mul i16 %a, %a
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @mul(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: mul:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0           ; 2-byte Folded Spill
; CHECK:         bal __mulhi3, $r0
; CHECK:         ldw $r0, [$r1, 0]           ; 2-byte Folded Reload
; CHECK:         addi $r1, $r1, 2
  %1 = mul i16 %a, %b
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @mul_constant(i16 %a) nounwind {
; CHECK-LABEL: mul_constant:
; CHECK:         subi $r1, $r1, 4
; CHECK:         stw [$r1, 2], $r0           ; 2-byte Folded Spill
; CHECK:         stw [$r1, 0], $r3           ; 2-byte Folded Spill
; CHECK:         movi $r3, 5
; CHECK:         bal __mulhi3, $r0
; CHECK:         ldw $r3, [$r1, 0]           ; 2-byte Folded Reload
; CHECK:         ldw $r0, [$r1, 2]           ; 2-byte Folded Reload
; CHECK:         addi $r1, $r1, 4
  %1 = mul i16 %a, 5
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i16 @mul_pow2(i16 %a) nounwind {
; CHECK-LABEL: mul_pow2:
; CHECK:         lsli $r2, $r2, 3
  %1 = mul i16 %a, 8
  ret i16 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @mul32(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: mul32:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0           ; 2-byte Folded Spill
; CHECK:         bal __mulsi3, $r0
; CHECK:         ldw $r0, [$r1, 0]           ; 2-byte Folded Reload
; CHECK:         addi $r1, $r1, 2
  %1 = mul i32 %a, %b
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}

define i32 @mul32_constant(i32 %a) nounwind {
; CHECK-LABEL: mul32_constant:
; CHECK:         subi $r1, $r1, 6
; CHECK:         stw [$r1, 4], $r0           ; 2-byte Folded Spill
; CHECK:         stw [$r1, 2], $r4           ; 2-byte Folded Spill
; CHECK:         stw [$r1, 0], $r5           ; 2-byte Folded Spill
; CHECK:         movi $r4, 5
; CHECK:         movi $r5, 0
; CHECK:         bal __mulsi3, $r0
; CHECK:         ldw $r5, [$r1, 0]           ; 2-byte Folded Reload
; CHECK:         ldw $r4, [$r1, 2]           ; 2-byte Folded Reload
; CHECK:         ldw $r0, [$r1, 4]           ; 2-byte Folded Reload
; CHECK:         addi $r1, $r1, 6
  %1 = mul i32 %a, 5
  ret i32 %1 ; CHECK: jmp   {{.*JMP}}
}
