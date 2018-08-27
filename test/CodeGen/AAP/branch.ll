; RUN: llc -asm-show-inst -march=aap < %s | FileCheck %s


; Check the correctness of various conditional branch operations.

define void @foo(i16 %a, i16 *%b, i1 %c) {
; CHECK-LABEL: foo:
; CHECK:         ldw $[[REG1:r[0-9]+]], [$r3, 0]
; CHECK:         beq .LBB0_12, $[[REG1]], $r2
  %val1 = load volatile i16, i16* %b
  %tst1 = icmp eq i16 %val1, %a
  br i1 %tst1, label %end, label %test2

; CHECK:       ; %test2
; CHECK:         ldw $[[REG2:r[0-9]+]], [$r3, 0]
; CHECK:         bne .LBB0_12, $[[REG2]], $r2
test2:
  %val2 = load volatile i16, i16* %b
  %tst2 = icmp ne i16 %val2, %a
  br i1 %tst2, label %end, label %test3

; CHECK:       ; %test3
; CHECK:         ldw $[[REG3:r[0-9]+]], [$r3, 0]
; CHECK:         blts .LBB0_12, $[[REG3]], $r2
test3:
  %val3 = load volatile i16, i16* %b
  %tst3 = icmp slt i16 %val3, %a
  br i1 %tst3, label %end, label %test4

; CHECK:       ; %test4
; CHECK:         ldw $[[REG4:r[0-9]+]], [$r3, 0]
; CHECK:    bles .LBB0_12, $[[REG4]], $r2
test4:
  %val4 = load volatile i16, i16* %b
  %tst4 = icmp sle i16 %val4, %a
  br i1 %tst4, label %end, label %test5

; CHECK:       ; %test5
; CHECK:         ldw $[[REG5:r[0-9]+]], [$r3, 0]
; CHECK:         bltu .LBB0_12, $[[REG5]], $r2
test5:
  %val5 = load volatile i16, i16* %b
  %tst5 = icmp ult i16 %val5, %a
  br i1 %tst5, label %end, label %test6

; CHECK:       ; %test6
; CHECK:         ldw $[[REG6:r[0-9]+]], [$r3, 0]
; CHECK:         bleu .LBB0_12, $[[REG6]], $r2
test6:
  %val6 = load volatile i16, i16* %b
  %tst6 = icmp ule i16 %val6, %a
  br i1 %tst6, label %end, label %test7

; Check for condition codes that don't have a matching instruction

; CHECK:       ; %test7
; CHECK:         ldw $[[REG7:r[0-9]+]], [$r3, 0]
; CHECK:         blts .LBB0_12, $r2, $[[REG7]]
test7:
  %val7 = load volatile i16, i16* %b
  %tst7 = icmp sgt i16 %val7, %a
  br i1 %tst7, label %end, label %test8

; CHECK:       ; %test8
; CHECK:         ldw $[[REG8:r[0-9]+]], [$r3, 0]
; CHECK:         bles .LBB0_12, $r2, $[[REG8]]
test8:
  %val8 = load volatile i16, i16* %b
  %tst8 = icmp sge i16 %val8, %a
  br i1 %tst8, label %end, label %test9

; CHECK:       ; %test9
; CHECK:         ldw $[[REG9:r[0-9]+]], [$r3, 0]
; CHECK:         bltu .LBB0_12, $r2, $[[REG9]]
test9:
  %val9 = load volatile i16, i16* %b
  %tst9 = icmp ugt i16 %val9, %a
  br i1 %tst9, label %end, label %test10

; CHECK:       ; %test10
; CHECK:         ldw $[[REG10:r[0-9]+]], [$r3, 0]
; CHECK:         bleu .LBB0_12, $r2, $[[REG10]]
test10:
  %val10 = load volatile i16, i16* %b
  %tst10 = icmp uge i16 %val10, %a
  br i1 %tst10, label %end, label %test11

; CHECK:       ; %test11
; CHECK:         ldw $[[REG11:r[0-9]+]], [$r3, 0]
; CHECK:         andi $[[REG11:r[0-9]+]], $r4, 1
; CHECK:         movi $[[REGZ:r[0-9]+]], 0
; CHECK:         bne  .LBB0_12, $[[REG11]], $[[REGZ]]
test11:
  %val11 = load volatile i16, i16* %b
  br i1 %c, label %end, label %test12

; CHECK:       ; %test12
; CHECK:         ldw $[[REG11:r[0-9]+]], [$r3, 0]
test12:
  %val12 = load volatile i16, i16* %b
  br label %end

; CHECK:       .LBB0_12:
end:
  ret void ; CHECK: jmp   {{.*JMP}}
}
