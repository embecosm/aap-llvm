; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shorest)
; branch instructions
; Short branch instructions are never assembled

beq    511, $r0,  $r1   ;CHECK: {{BEQ_$}}
bne   -512, $r7,  $r0   ;CHECK: {{BNE_$}}
blts    15, $r13, $r61  ;CHECK: {{BLTS_$}}
bles    32, $r9,  $r8   ;CHECK: {{BLES_$}}
bltu   186, $r11, $r12  ;CHECK: {{BLTU_$}}
bleu    -1, $r17, $r19  ;CHECK: {{BLEU_$}}

bra      123    ;CHECK: {{BRA$}}
bra    -6810    ;CHECK: {{BRA$}}
bra  -131072    ;CHECK: {{BRA$}}
bra   131071    ;CHECK: {{BRA$}}

bal      481, $r14    ;CHECK: {{BAL$}}
bal    -1892, $r58    ;CHECK: {{BAL$}}
bal    32767, $r0     ;CHECK: {{BAL$}}
bal   -32768, $r15    ;CHECK: {{BAL$}}
