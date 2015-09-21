; RUN: llvm-mc -triple=aap -show-inst %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; alu operations.


; ALU operations with registers
add   $r0,  $r0,  $r0   ;CHECK: {{ADD_r_short}}
add   $r1,  $r2,  $r7   ;CHECK: {{ADD_r_short}}
add   $r7,  $r7,  $r7   ;CHECK: {{ADD_r_short}}
add   $r8,  $r0,  $r5   ;CHECK: {{ADD_r$}}
add   $r8,  $r8,  $r12  ;CHECK: {{ADD_r$}}
add   $r2,  $r54, $r8   ;CHECK: {{ADD_r$}}
add   $r1,  $r32, $r12  ;CHECK: {{ADD_r$}}

xor   $r0,  $r0,  $r0   ;CHECK: {{XOR_r_short}}
and   $r1,  $r2,  $r7   ;CHECK: {{AND_r_short}}
or    $r7,  $r7,  $r1   ;CHECK: {{OR_r_short}}
sub   $r9,  $r1,  $r5   ;CHECK: {{SUB_r$}}
addc  $r9,  $r9,  $r12  ;CHECK: {{ADDC_r$}}
subc  $r4,  $r55, $r8   ;CHECK: {{SUBC_r$}}
asr   $r4,  $r33, $r12  ;CHECK: {{ASR_r$}}
lsr   $r15, $r3,  $r8   ;CHECK: {{LSR_r$}}
lsl   $r11, $r10, $r12  ;CHECK: {{LSL_r$}}


; Add/Sub with immediates
addi  $r6,  $r4,  0     ;CHECK: {{ADDI_i3_short}}
addi  $r7,  $r2,  1     ;CHECK: {{ADDI_i3_short}}
subi  $r0,  $r5,  3     ;CHECK: {{SUBI_i3_short}}
subi  $r8,  $r8,  7     ;CHECK: {{SUBI_i10}}
addi  $r7,  $r7,  1023  ;CHECK: {{ADDI_i10}}

; Add/Sub with expressions
addi  $r0,  $r0,  (0 + 0)         ;CHECK: {{ADDI_i3_short}}
subi  $r7,  $r7,  (1024 - 1023)   ;CHECK: {{SUBI_i3_short}}
addi  $r1,  $r5,  (8 - 1)         ;CHECK: {{ADDI_i3_short}}
subi  $r3,  $r0,  (7 + 1)         ;CHECK: {{SUBI_i10}}

; Add/Sub with relocs
addi  $r5,  $r2,  a     ;CHECK: {{ADDI_i10}}
subi  $r7,  $r0,  b     ;CHECK: {{SUBI_i10}}


; Logical operation with immediates
andi  $r0,  $r2,  7   ;CHECK: {{ANDI_i9}}
ori   $r1,  $r5,  0   ;CHECK: {{ORI_i9}}
xori  $r6,  $r7,  6   ;CHECK: {{XORI_i9}}
andi  $r8,  $r0,  123 ;CHECK: {{ANDI_i9}}
ori   $r5,  $r8,  12  ;CHECK: {{ORI_i9}}
xori  $r15, $r54, 15  ;CHECK: {{XORI_i9}}
andi  $r9,  $r14, 511 ;CHECK: {{ANDI_i9}}

; Logical operations with expressions
andi  $r0,  $r1,  (5 - 3)   ;CHECK: {{ANDI_i9}}
xori  $r15, $r12, (0 - 0)   ;CHECK: {{XORI_i9}}
ori   $r43, $r19, (15 + 48) ;CHECK: {{ORI_i9}}

; Logical operations with relocations
andi  $r5,  $r1,  a   ;CHECK: {{ANDI_i9}}
xori  $r11, $r5,  a+b ;CHECK: {{XORI_i9}}
ori   $r0,  $r0,  c-a ;CHECK: {{ORI_i9}}


; Shift operations with immediates
lsli  $r0,  $r0,  1   ;CHECK: {{LSLI_i3_short}}
lsri  $r4,  $r0,  2   ;CHECK: {{LSRI_i3_short}}
asri  $r7,  $r1,  8   ;CHECK: {{ASRI_i3_short}}
asri  $r7,  $r3,  5   ;CHECK: {{ASRI_i3_short}}

lsli  $r0,  $r8,  1   ;CHECK: {{LSLI_i6$}}
lsli  $r8,  $r5,  3   ;CHECK: {{LSLI_i6$}}
lsri  $r31, $r15, 4   ;CHECK: {{LSRI_i6$}}
asri  $r7,  $r35, 1   ;CHECK: {{ASRI_i6$}}

lsli  $r2,  $r4,  9   ;CHECK: {{LSLI_i6$}}
lsri  $r8,  $r0,  12  ;CHECK: {{LSRI_i6$}}
asri  $r7,  $r1,  34  ;CHECK: {{ASRI_i6$}}
asri  $r7,  $r3,  63  ;CHECK: {{ASRI_i6$}}

; Shift operations with expressions
lsri  $r4,  $r2,  (3 + 2)     ;CHECK: {{LSRI_i3_short}}
asri  $r1,  $r1,  (1 + 0)     ;CHECK: {{ASRI_i3_short}}
lsli  $r2,  $r7,  (128 - 65)  ;CHECK: {{LSLI_i6$}}
lsri  $r8,  $r0,  (1 + 2)     ;CHECK: {{LSRI_i6$}}

; Shift operations with relocations
lsli  $r0,  $r5,  a   ;CHECK: {{LSLI_i6$}}
lsri  $r0,  $r5,  b   ;CHECK: {{LSRI_i6$}}
asri  $r0,  $r5,  c   ;CHECK: {{ASRI_i6$}}
