; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; load instructions.

; Byte loads with immediates
ldb   $r7,  [$r1,   0]    ;CHECK: encoding: [0xc8,0x21]
ldb   $r0,  [$r0,   2]    ;CHECK: encoding: [0x02,0x20]
ldb   $r7,  [$r7,   3]    ;CHECK: encoding: [0xfb,0x21]
ldb   $r4,  [$r1,  -1]    ;CHECK: encoding: [0x0f,0x21]
ldb   $r6,  [$r0,  -4]    ;CHECK: encoding: [0x84,0x21]
ldb   $r5,  [$r8,   0]    ;CHECK: encoding: [0x40,0xa1,0x08,0x00]
ldb   $r16, [$r1,   2]    ;CHECK: encoding: [0x0a,0xa0,0x80,0x00]
ldb   $r1,  [$r2,   4]    ;CHECK: encoding: [0x54,0xa0,0x00,0x00]
ldb   $r6,  [$r3,  -5]    ;CHECK: encoding: [0x9b,0xa1,0x07,0x1e]
ldb   $r43, [$r56, 12]    ;CHECK: encoding: [0xc4,0xa0,0x79,0x01]
ldb   $r11, [$r11,  511]  ;CHECK: encoding: [0xdf,0xa0,0x4f,0x0e]
ldb   $r53, [$r63, -512]  ;CHECK: encoding: [0x78,0xa1,0xb8,0x11]

; Byte loads with expressions
ldb   $r0,  [$r4,   (5 - 3)]          ;CHECK: encoding: [0x22,0x20]
ldb   $r5,  [$r7,   (81726 - 81727)]  ;CHECK: encoding: [0x7f,0x21]
ldb   $r2,  [$r6,   (-5 + 1)]         ;CHECK: encoding: [0xb4,0x20]
ldb   $r15, [$r34,  (0 + 0)]          ;CHECK: encoding: [0xd0,0xa1,0x60,0x00]
ldb   $r45, [$r0,   (5 - 2)]          ;CHECK: encoding: [0x43,0xa1,0x40,0x01]
ldb   $r0,  [$r61,  (11 - 9)]         ;CHECK: encoding: [0x2a,0xa0,0x38,0x00]
ldb   $r5,  [$r1,   (256 + 255)]      ;CHECK: encoding: [0x4f,0xa1,0x07,0x0e]
ldb   $r43, [$r17,  (-256 - 256)]     ;CHECK: encoding: [0xc8,0xa0,0x50,0x11]

; Byte loads with relocations
ldb   $r0,  [$r5,   a]        ;CHECK: encoding: [0b00101AAA,0xa0,0x00,0x00]
ldb   $r1,  [$r3,   b]        ;CHECK: encoding: [0b01011AAA,0xa0,0x00,0x00]
ldb   $r17, [$r39,  b]        ;CHECK: encoding: [0b01111AAA,0xa0,0xa0,0x00]
ldb   $r15, [$r11,  (a - b)]  ;CHECK: encoding: [0b11011AAA,0xa1,0x48,0x00]

; Postinc/Predec bytes loads with immediate
ldb   $r5,  [$r4+,   0]    ;CHECK: encoding: [0x60,0x23]
ldb   $r0,  [$r2+,   2]    ;CHECK: encoding: [0x12,0x22]
ldb   $r1,  [-$r6,   2]    ;CHECK: encoding: [0x72,0x24]
ldb   $r7,  [-$r2,  -2]    ;CHECK: encoding: [0xd6,0x25]
ldb   $r1,  [-$r7,  -4]    ;CHECK: encoding: [0x7c,0x24]
ldb   $r5,  [$r8+,   3]    ;CHECK: encoding: [0x43,0xa3,0x08,0x00]
ldb   $r11, [$r1+,   1]    ;CHECK: encoding: [0xc9,0xa2,0x40,0x00]
ldb   $r3,  [-$r5,   5]    ;CHECK: encoding: [0xed,0xa4,0x00,0x00]
ldb   $r2,  [-$r3,  -5]    ;CHECK: encoding: [0x9b,0xa4,0x07,0x1e]
ldb   $r41, [$r56+, 12]    ;CHECK: encoding: [0x44,0xa2,0x79,0x01]
ldb   $r34, [$r11+,  511]  ;CHECK: encoding: [0x9f,0xa2,0x0f,0x0f]
ldb   $r60, [$r63+, -512]  ;CHECK: encoding: [0x38,0xa3,0xf8,0x11]

; Postinc/Predec byte loads with expressions
ldb   $r1,  [$r7+,  (5 - 3)]          ;CHECK: encoding: [0x7a,0x22]
ldb   $r5,  [-$r1,  (81726 - 81727)]  ;CHECK: encoding: [0x4f,0x25]
ldb   $r7,  [-$r6,  (-5 + 1)]         ;CHECK: encoding: [0xf4,0x25]
ldb   $r29, [$r34+, (0 + 0)]          ;CHECK: encoding: [0x50,0xa3,0xe0,0x00]
ldb   $r40, [-$r1,  (5 - 2)]          ;CHECK: encoding: [0x0b,0xa4,0x40,0x01]
ldb   $r0,  [$r61+, (11 - 9)]         ;CHECK: encoding: [0x2a,0xa2,0x38,0x00]
ldb   $r5,  [$r1+,  (256 + 255)]      ;CHECK: encoding: [0x4f,0xa3,0x07,0x0e]
ldb   $r43, [$r17+, (-256 - 256)]     ;CHECK: encoding: [0xc8,0xa2,0x50,0x11]

; Postinc/Predec byte loads with relocations
ldb   $r0,  [$r5+,   a]        ;CHECK: encoding: [0b00101AAA,0xa2,0x00,0x00]
ldb   $r1,  [-$r3,   b]        ;CHECK: encoding: [0b01011AAA,0xa4,0x00,0x00]
ldb   $r17, [$r39+,  b]        ;CHECK: encoding: [0b01111AAA,0xa2,0xa0,0x00]
ldb   $r15, [-$r11,  (a - b)]  ;CHECK: encoding: [0b11011AAA,0xa5,0x48,0x00]


; Word loads with immediates
ldw   $r7,  [$r1,   0]    ;CHECK: encoding: [0xc8,0x29]
ldw   $r4,  [$r1,  -1]    ;CHECK: encoding: [0x0f,0x29]
ldw   $r6,  [$r0,  -4]    ;CHECK: encoding: [0x84,0x29]
ldw   $r5,  [$r8,   0]    ;CHECK: encoding: [0x40,0xa9,0x08,0x00]
ldw   $r6,  [$r3,  -5]    ;CHECK: encoding: [0x9b,0xa9,0x07,0x1e]
ldw   $r53, [$r63, -512]  ;CHECK: encoding: [0x78,0xa9,0xb8,0x11]

; Word loads with expressions
ldw   $r0,  [$r4,   (5 - 3)]          ;CHECK: encoding: [0x22,0x28]
ldw   $r5,  [$r7,   (81726 - 81727)]  ;CHECK: encoding: [0x7f,0x29]
ldw   $r15, [$r34,  (0 + 0)]          ;CHECK: encoding: [0xd0,0xa9,0x60,0x00]
ldw   $r43, [$r17,  (-256 - 256)]     ;CHECK: encoding: [0xc8,0xa8,0x50,0x11]

; Word loads with relocations
ldw   $r17, [$r39,  b]        ;CHECK: encoding: [0b01111AAA,0xa8,0xa0,0x00]
ldw   $r15, [$r11,  (a - b)]  ;CHECK: encoding: [0b11011AAA,0xa9,0x48,0x00]

; Postinc/Predec word loads with immediates
ldw   $r0,  [$r2+,   2]    ;CHECK: encoding: [0x12,0x2a]
ldw   $r7,  [-$r2,  -2]    ;CHECK: encoding: [0xd6,0x2d]
ldw   $r1,  [-$r7,  -4]    ;CHECK: encoding: [0x7c,0x2c]
ldw   $r11, [$r1+,   1]    ;CHECK: encoding: [0xc9,0xaa,0x40,0x00]
ldw   $r2,  [-$r3,  -5]    ;CHECK: encoding: [0x9b,0xac,0x07,0x1e]
ldw   $r60, [$r63+, -512]  ;CHECK: encoding: [0x38,0xab,0xf8,0x11]

; Postinc/Predec word loads with expressions
ldw   $r1,  [$r7+,  (5 - 3)]      ;CHECK: encoding: [0x7a,0x2a]
ldw   $r7,  [-$r6,  (-5 + 1)]     ;CHECK: encoding: [0xf4,0x2d]
ldw   $r29, [$r34+, (0 + 0)]      ;CHECK: encoding: [0x50,0xab,0xe0,0x00]
ldw   $r40, [-$r1,  (5 - 2)]      ;CHECK: encoding: [0x0b,0xac,0x40,0x01]
ldw   $r43, [$r17+, (-256 - 256)] ;CHECK: encoding: [0xc8,0xaa,0x50,0x11]

; Postinc/Predec word loads with relocations
ldw   $r17, [$r39+,  b]        ;CHECK: encoding: [0b01111AAA,0xaa,0xa0,0x00]
ldw   $r15, [-$r11,  (a - b)]  ;CHECK: encoding: [0b11011AAA,0xad,0x48,0x00]
