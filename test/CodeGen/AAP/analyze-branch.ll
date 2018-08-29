; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check that AAP can do basic analysis of branches and manipulation
; of basic blocks

declare void @test_true()
declare void @test_false()

; !0 corresponds to a branch being taken, !1 to not being takne.
!0 = !{!"branch_weights", i16 64, i16 4}
!1 = !{!"branch_weights", i16 4, i16 64}

define void @test_bcc_fallthrough_taken(i16 %in) nounwind {
; CHECK-LABEL: test_bcc_fallthrough_taken:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0           ; 2-byte Folded Spill
; CHECK:         movi $r10, 42
; CHECK:         bne .LBB0_3, $r2, $r10
  %tst = icmp eq i16 %in, 42
  br i1 %tst, label %true, label %false, !prof !0

; Expected layout order is: Entry, TrueBlock, FalseBlock
; Entry->TrueBlock is the common path, which should be taken whenever the
; conditional branch is false.

true:
; CHECK:         bal test_true, $r0
  call void @test_true()

; CHECK:       .LBB0_2
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
  ret void ; CHECK: jmp   {{.*JMP}}

false:
; CHECK:       .LBB0_3
; CHECK:         bal test_false, $r0
; CHECK:         bra .LBB0_2
  call void @test_false()
  ret void
}

define void @test_bcc_fallthrough_nottaken(i16 %in) nounwind {
; CHECK-LABEL: test_bcc_fallthrough_nottaken:
; CHECK:         subi $r1, $r1, 2
; CHECK:         stw [$r1, 0], $r0
; CHECK:         movi $r10, 42
; CHECK:         beq .LBB1_1, $r2, $r10
;
; CHECK:         bal test_false, $r0
;
; CHECK:       .LBB1_2:
; CHECK:         ldw $r0, [$r1, 0]
; CHECK:         addi $r1, $r1, 2
; CHECK:         jmp $r0
;
; CHECK:       .LBB1_1:
; CHECK:         bal test_true, $r0
; CHECK:         bra .LBB1_2
  %tst = icmp eq i16 %in, 42
  br i1 %tst, label %true, label %false, !prof !1

; Expected layout order is: Entry, FalseBlock, TrueBlock
; Entry->FalseBlock is the common path, which should be taken whenever the
; conditional branch is false

true:
  call void @test_true()
  ret void

false:
  call void @test_false()
  ret void
}
