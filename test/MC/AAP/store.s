; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; store instructions.

; Byte stores with immediates
stb   [$r0,   0],   $r0   ;CHECK: {{STB_short}}
stb   [$r1,   2],   $r1   ;CHECK: {{STB_short}}
stb   [$r7,   3],   $r3   ;CHECK: {{STB_short}}
stb   [$r1,  -1],   $r7   ;CHECK: {{STB_short}}
stb   [$r5,  -4],   $r5   ;CHECK: {{STB_short}}
stb   [$r8,   0],   $r0   ;CHECK: {{STB$}}
stb   [$r1,   4],   $r0   ;CHECK: {{STB$}}
stb   [$r7,   3],   $r8   ;CHECK: {{STB$}}
stb   [$r15, -1],   $r7   ;CHECK: {{STB$}}
stb   [$r6,  -5],   $r2   ;CHECK: {{STB$}}
stb   [$r63,  2],   $r7   ;CHECK: {{STB$}}
stb   [$r5,   1],   $r26  ;CHECK: {{STB$}}
stb   [$r63,  511], $r19  ;CHECK: {{STB$}}
stb   [$r52, -512], $r14  ;CHECK: {{STB$}}

; Byte stores with expressions
stb   [$r1,   (-(3 - 2))],        $r2   ;CHECK: {{STB_short}}
stb   [$r4,   (1234 - 1233)],     $r7   ;CHECK: {{STB_short}}
stb   [$r6,   (-2 + 1)],          $r2   ;CHECK: {{STB_short}}
stb   [$r1,   (1024 - 513)],      $r0   ;CHECK: {{STB$}}
stb   [$r40,  (-256 + 123)],      $r16  ;CHECK: {{STB$}}
stb   [$r18,  (0xdead - 0xdeaf)], $r8   ;CHECK: {{STB$}}

; Byte stores with relocations
stb   [$r0,  a],        $r1   ;CHECK: {{STB$}}
stb   [$r7,  b],        $r2   ;CHECK: {{STB$}}
stb   [$r1,  (c - a)],  $r50  ;CHECK: {{STB$}}

; Postinc/Predec byte stores with immediates
stb   [$r5+,   1],   $r0   ;CHECK: {{STB_postinc_short}}
stb   [$r6+,   3],   $r2   ;CHECK: {{STB_postinc_short}}
stb   [-$r7,   2],   $r7   ;CHECK: {{STB_predec_short}}
stb   [-$r1,  -1],   $r7   ;CHECK: {{STB_predec_short}}
stb   [$r2+,  -4],   $r2   ;CHECK: {{STB_postinc_short}}
stb   [$r9+,   1],   $r8   ;CHECK: {{STB_postinc$}}
stb   [$r1+,   4],   $r0   ;CHECK: {{STB_postinc$}}
stb   [-$r7,  -5],   $r8   ;CHECK: {{STB_predec$}}
stb   [-$r12, -1],   $r7   ;CHECK: {{STB_predec$}}
stb   [-$r61,  -5],  $r2   ;CHECK: {{STB_predec$}}
stb   [$r25+,  2],   $r7   ;CHECK: {{STB_postinc$}}
stb   [$r1+,   1],   $r29  ;CHECK: {{STB_postinc$}}
stb   [-$r27,  511], $r19  ;CHECK: {{STB_predec$}}
stb   [$r52+, -512], $r44  ;CHECK: {{STB_postinc$}}

; Postinc/Predec byte stores with expressions
stb   [$r0+,   (-(-5 + 4))],       $r0   ;CHECK: {{STB_postinc_short}}
stb   [-$r2,   (91 - 95)],         $r2   ;CHECK: {{STB_predec_short}}
stb   [$r6+,   (2 - 1)],           $r2   ;CHECK: {{STB_postinc_short}}
stb   [-$r1,   (1024 - 513)],      $r8   ;CHECK: {{STB_predec$}}
stb   [$r40+,  (-256 + 256)],      $r16  ;CHECK: {{STB_postinc$}}
stb   [-$r18,  (0xdead - 0xdead)], $r8   ;CHECK: {{STB_predec$}}

; Postinc/Predec byte stores with relocations
stb   [$r7+,  a],        $r6   ;CHECK: {{STB_postinc$}}
stb   [-$r0,  b],        $r3   ;CHECK: {{STB_predec$}}
stb   [$r1+,  (b + a)],  $r8   ;CHECK: {{STB_postinc$}}


; Word stores with immediates
stb   [$r1,   2],   $r1   ;CHECK: {{STB_short}}
stb   [$r7,   3],   $r3   ;CHECK: {{STB_short}}
stb   [$r8,   0],   $r0   ;CHECK: {{STB$}}
stb   [$r6,  -5],   $r2   ;CHECK: {{STB$}}
stb   [$r5,   1],   $r26  ;CHECK: {{STB$}}

; Word stores with expressions
stb   [$r4,   (1234 - 1233)],     $r7   ;CHECK: {{STB_short}}
stb   [$r6,   (-2 + 1)],          $r2   ;CHECK: {{STB_short}}
stb   [$r1,   (1024 - 513)],      $r0   ;CHECK: {{STB$}}
stb   [$r18,  (0xdead - 0xdeaf)], $r8   ;CHECK: {{STB$}}

; Word stores with relocations
stb   [$r7,  b],        $r2   ;CHECK: {{STB$}}
stb   [$r1,  (c - a)],  $r50  ;CHECK: {{STB$}}

; Postinc/Predec word stores with immediates
stb   [-$r7,   2],   $r7   ;CHECK: {{STB_predec_short}}
stb   [$r2+,  -4],   $r2   ;CHECK: {{STB_postinc_short}}
stb   [$r9+,   1],   $r8   ;CHECK: {{STB_postinc$}}
stb   [-$r7,  -5],   $r8   ;CHECK: {{STB_predec$}}
stb   [-$r61,  -5],  $r2   ;CHECK: {{STB_predec$}}
stb   [$r25+,  2],   $r7   ;CHECK: {{STB_postinc$}}

; Postinc/Predec word stores with expressions
stb   [-$r2,   (91 - 95)],        $r3   ;CHECK: {{STB_predec_short}}
stb   [$r6+,   (2 - 1)],          $r0   ;CHECK: {{STB_postinc_short}}
stb   [$r4+,  (-256 + 256)],      $r53  ;CHECK: {{STB_postinc$}}
stb   [-$r8,  (0xdead - 0xdead)], $r8   ;CHECK: {{STB_predec$}}

; Postinc/Predec word stores with relocations
stb   [-$r7,  b],        $r3   ;CHECK: {{STB_predec$}}
stb   [$r3+,  (a + b + c + d)],  $r8   ;CHECK: {{STB_postinc$}}
