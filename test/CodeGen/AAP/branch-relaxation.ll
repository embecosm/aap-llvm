; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of the branch relaxation pass.

; Check relaxation of BEQ, which can be reversed to BNE, IE:
;
;   beq after, $r2, $r3
; before:
;   ...
;
; Can be relaxed to:
;
;   bne before, $r2, $r3
;   bra after
; before:
;   ...
define i32 @rel_beq(i32 %a) {
entry:
;CHECK:     rel_beq:
;CHECK:       bne .LBB0_1, ${{r[0-9]+}}, ${{r[0-9]+}}
;CHECK:       bra .LBB0_2
  %cmp = icmp eq i32 %a, 0
  br i1 %cmp, label %return, label %if.end

if.end:
  tail call void asm sideeffect ".space 2048", ""()
  br label %return

return:
  %retval.0 = phi i32 [ 0, %if.end ], [ -1, %entry ]
  ret i32 %retval.0 ;CHECK: jmp   {{.*PseudoRET}}
}

; Check relaxation of BLTS, which does not have a reverse condition and so
; requires a new block to complete the long branch, IE:
;
;   blts after, $r2, $r3
; before:
;   ...
;
; Can be relaxed to:
;
;   blts step, $r2, $r3
;   bra before
; step:
;   bra after
; before:
;   ...
define i32 @rel_blts(i32 %a) {
entry:
;CHECK:     rel_blts:
;CHECK:       blts .LBB1_3, ${{r[0-9]+}}, ${{r[0-9]+}}
;CHECK:       bra  .LBB1_1
  %cmp = icmp slt i32 %a, 0
  br i1 %cmp, label %return, label %if.end

;CHECK:     .LBB1_3:
;CHECK:       bra .LBB1_2

if.end:
;CHECK:     .LBB1_1:
  tail call void asm sideeffect ".space 2048", ""()
  br label %return

return:
;CHECK:     .LBB1_2:
  %retval.0 = phi i32 [ 0, %if.end ], [ -1, %entry ]
  ret i32 %retval.0 ;CHECK: jmp   {{.*PseudoRET}}
}
