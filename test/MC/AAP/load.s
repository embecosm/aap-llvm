; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; load instructions.

; Byte loads with immediates
ldb   $r7,  [$r1,   0]    ;CHECK: {{LDB_short}}
ldb   $r0,  [$r0,   2]    ;CHECK: {{LDB_short}}
ldb   $r7,  [$r7,   3]    ;CHECK: {{LDB_short}}
ldb   $r4,  [$r1,  -1]    ;CHECK: {{LDB_short}}
ldb   $r6,  [$r0,  -4]    ;CHECK: {{LDB_short}}
ldb   $r5,  [$r8,   0]    ;CHECK: {{LDB$}}
ldb   $r16, [$r1,   2]    ;CHECK: {{LDB$}}
ldb   $r1,  [$r2,   4]    ;CHECK: {{LDB$}}
ldb   $r6,  [$r3,  -5]    ;CHECK: {{LDB$}}
ldb   $r43, [$r56, 12]    ;CHECK: {{LDB$}} 
ldb   $r11, [$r11,  511]  ;CHECK: {{LDB$}}
ldb   $r53, [$r63, -512]  ;CHECK: {{LDB$}}

; Byte loads with expressions
ldb   $r0,  [$r4,   (5 - 3)]          ;CHECK: {{LDB_short}}
ldb   $r5,  [$r7,   (81726 - 81727)]  ;CHECK: {{LDB_short}}
ldb   $r2,  [$r6,   (-5 + 1)]         ;CHECK: {{LDB_short}}
ldb   $r15, [$r34,  (0 + 0)]          ;CHECK: {{LDB$}}
ldb   $r45, [$r0,   (5 - 2)]          ;CHECK: {{LDB$}}
ldb   $r0,  [$r61,  (11 - 9)]         ;CHECK: {{LDB$}}
ldb   $r5,  [$r1,   (256 + 255)]      ;CHECK: {{LDB$}}
ldb   $r43, [$r17,  (-256 - 256)]     ;CHECK: {{LDB$}}

; Byte loads with relocations
ldb   $r0,  [$r5,   a]        ;CHECK: {{LDB$}}
ldb   $r1,  [$r3,   b]        ;CHECK: {{LDB$}}
ldb   $r17, [$r39,  b]        ;CHECK: {{LDB$}}
ldb   $r15, [$r11,  (a - b)]  ;CHECK: {{LDB$}}

; Postinc/Predec bytes loads with immediate
ldb   $r5,  [$r4+,   0]    ;CHECK: {{LDB_postinc_short}}
ldb   $r0,  [$r2+,   2]    ;CHECK: {{LDB_postinc_short}}
ldb   $r1,  [-$r6,   2]    ;CHECK: {{LDB_predec_short}}
ldb   $r7,  [-$r2,  -2]    ;CHECK: {{LDB_predec_short}}
ldb   $r1,  [-$r7,  -4]    ;CHECK: {{LDB_predec_short}}
ldb   $r5,  [$r8+,   3]    ;CHECK: {{LDB_postinc$}}
ldb   $r11, [$r1+,   1]    ;CHECK: {{LDB_postinc$}}
ldb   $r3,  [-$r5,   5]    ;CHECK: {{LDB_predec$}}
ldb   $r2,  [-$r3,  -5]    ;CHECK: {{LDB_predec$}}
ldb   $r41, [$r56+, 12]    ;CHECK: {{LDB_postinc$}} 
ldb   $r34, [$r11+,  511]  ;CHECK: {{LDB_postinc$}}
ldb   $r60, [$r63+, -512]  ;CHECK: {{LDB_postinc$}}

; Postinc/Predec byte loads with expressions
ldb   $r1,  [$r7+,  (5 - 3)]          ;CHECK: {{LDB_postinc_short}}
ldb   $r5,  [-$r1,  (81726 - 81727)]  ;CHECK: {{LDB_predec_short}}
ldb   $r7,  [-$r6,  (-5 + 1)]         ;CHECK: {{LDB_predec_short}}
ldb   $r29, [$r34+, (0 + 0)]          ;CHECK: {{LDB_postinc$}}
ldb   $r40, [-$r1,  (5 - 2)]          ;CHECK: {{LDB_predec$}}
ldb   $r0,  [$r61+, (11 - 9)]         ;CHECK: {{LDB_postinc$}}
ldb   $r5,  [$r1+,  (256 + 255)]      ;CHECK: {{LDB_postinc$}}
ldb   $r43, [$r17+, (-256 - 256)]     ;CHECK: {{LDB_postinc$}}

; Postinc/Predec byte loads with relocations
ldb   $r0,  [$r5+,   a]        ;CHECK: {{LDB_postinc$}}
ldb   $r1,  [-$r3,   b]        ;CHECK: {{LDB_predec$}}
ldb   $r17, [$r39+,  b]        ;CHECK: {{LDB_postinc$}}
ldb   $r15, [-$r11,  (a - b)]  ;CHECK: {{LDB_predec$}}


; Word loads with immediates
ldw   $r7,  [$r1,   0]    ;CHECK: {{LDW_short}}
ldw   $r4,  [$r1,  -1]    ;CHECK: {{LDW_short}}
ldw   $r6,  [$r0,  -4]    ;CHECK: {{LDW_short}}
ldw   $r5,  [$r8,   0]    ;CHECK: {{LDW$}}
ldw   $r6,  [$r3,  -5]    ;CHECK: {{LDW$}}
ldw   $r53, [$r63, -512]  ;CHECK: {{LDW$}}

; Word loads with expressions
ldw   $r0,  [$r4,   (5 - 3)]          ;CHECK: {{LDW_short}}
ldw   $r5,  [$r7,   (81726 - 81727)]  ;CHECK: {{LDW_short}}
ldw   $r15, [$r34,  (0 + 0)]          ;CHECK: {{LDW$}}
ldw   $r43, [$r17,  (-256 - 256)]     ;CHECK: {{LDW$}}

; Word loads with relocations
ldw   $r17, [$r39,  b]        ;CHECK: {{LDW$}}
ldw   $r15, [$r11,  (a - b)]  ;CHECK: {{LDW$}}

; Postinc/Predec word loads with immediates
ldw   $r0,  [$r2+,   2]    ;CHECK: {{LDW_postinc_short}}
ldw   $r7,  [-$r2,  -2]    ;CHECK: {{LDW_predec_short}}
ldw   $r1,  [-$r7,  -4]    ;CHECK: {{LDW_predec_short}}
ldw   $r11, [$r1+,   1]    ;CHECK: {{LDW_postinc$}}
ldw   $r2,  [-$r3,  -5]    ;CHECK: {{LDW_predec$}}
ldw   $r60, [$r63+, -512]  ;CHECK: {{LDW_postinc$}}

; Postinc/Predec word loads with expressions
ldw   $r1,  [$r7+,  (5 - 3)]          ;CHECK: {{LDW_postinc_short}}
ldw   $r7,  [-$r6,  (-5 + 1)]         ;CHECK: {{LDW_predec_short}}
ldw   $r29, [$r34+, (0 + 0)]          ;CHECK: {{LDW_postinc$}}
ldw   $r40, [-$r1,  (5 - 2)]          ;CHECK: {{LDW_predec$}}
ldw   $r43, [$r17+, (-256 - 256)]     ;CHECK: {{LDW_postinc$}}

; Postinc/Predec word loads with relocations
ldw   $r17, [$r39+,  b]        ;CHECK: {{LDW_postinc$}}
ldw   $r15, [-$r11,  (a - b)]  ;CHECK: {{LDW_predec$}}
