; RUN: llvm-mc -triple=aap -show-encoding %s | FileCheck %s

; Basic tests to check that we always pick the correct (and shortest)
; alu operations.


; ALU operations with registers
add   $r0,  $r0,  $r0   ;CHECK: add $r0, $r0,  $r0  ; encoding: [0x00,0x02]
add   $r1,  $r2,  $r7   ;CHECK: add $r1, $r2,  $r7  ; encoding: [0x57,0x02]
add   $r7,  $r7,  $r7   ;CHECK: add $r7, $r7,  $r7  ; encoding: [0xff,0x03]
add   $r8,  $r0,  $r5   ;CHECK: add $r8, $r0,  $r5  ; encoding: [0x05,0x82,0x40,0x00]
add   $r8,  $r8,  $r12  ;CHECK: add $r8, $r8,  $r12 ; encoding: [0x04,0x82,0x49,0x00]
add   $r2,  $r54, $r8   ;CHECK: add $r2, $r54, $r8  ; encoding: [0xb0,0x82,0x31,0x00]
add   $r1,  $r32, $r12  ;CHECK: add $r1, $r32, $r12 ; encoding: [0x44,0x82,0x21,0x00]

xor   $r0,  $r0,  $r0   ;CHECK: xor  $r0,  $r0,  $r0  ; encoding: [0x00,0x0a]
and   $r1,  $r2,  $r7   ;CHECK: and  $r1,  $r2,  $r7  ; encoding: [0x57,0x06]
or    $r7,  $r7,  $r1   ;CHECK: or   $r7,  $r7,  $r1  ; encoding: [0xf9,0x09]
sub   $r9,  $r1,  $r5   ;CHECK: sub  $r9,  $r1,  $r5  ; encoding: [0x4d,0x84,0x40,0x00]
addc  $r9,  $r9,  $r12  ;CHECK: addc $r9,  $r9,  $r12 ; encoding: [0x4c,0x82,0x49,0x02]
subc  $r4,  $r55, $r8   ;CHECK: subc $r4,  $r55, $r8  ; encoding: [0x38,0x85,0x31,0x02]
asr   $r4,  $r33, $r12  ;CHECK: asr  $r4,  $r33, $r12 ; encoding: [0x0c,0x8d,0x21,0x00]
lsr   $r15, $r3,  $r8   ;CHECK: lsr  $r15, $r3,  $r8  ; encoding: [0xd8,0x91,0x41,0x00]
lsl   $r11, $r10, $r12  ;CHECK: lsl  $r11, $r10, $r12 ; encoding: [0xd4,0x8e,0x49,0x00]


; Add/Sub with immediates
addi  $r6,  $r4,  0     ;CHECK: addi $r6,  $r4,  0    ; encoding: [0xa0,0x15]
addi  $r7,  $r2,  1     ;CHECK: addi $r7,  $r2,  1    ; encoding: [0xd1,0x15]
subi  $r0,  $r5,  3     ;CHECK: subi $r0,  $r5,  3    ; encoding: [0x2b,0x16]
subi  $r8,  $r8,  7     ;CHECK: subi $r8,  $r8,  7    ; encoding: [0x07,0x96,0x48,0x00]
addi  $r7,  $r7,  1023  ;CHECK: addi $r7,  $r7,  1023 ; encoding: [0xff,0x95,0x07,0x1e]

; Add/Sub with expressions
addi  $r0,  $r0,  (0 + 0)         ;CHECK: addi $r0, $r0, 0 ; encoding: [0x00,0x14]
subi  $r7,  $r7,  (1024 - 1023)   ;CHECK: subi $r7, $r7, 1 ; encoding: [0xf9,0x17]
addi  $r1,  $r5,  (8 - 1)         ;CHECK: addi $r1, $r5, 7 ; encoding: [0x6f,0x14]
subi  $r3,  $r0,  (7 + 1)         ;CHECK: subi $r3, $r0, 8 ; encoding: [0xc0,0x96,0x01,0x00]

; Add/Sub with relocs
addi  $r5,  $r2,  a     ;CHECK: addi $r5, $r2, a ; encoding: [0b01010AAA,0x95,0x00,0x00]
subi  $r7,  $r0,  b     ;CHECK: subi $r7, $r0, b ; encoding: [0b11000AAA,0x97,0x00,0x00]


; Logical operation with immediates
andi  $r0,  $r2,  7   ;CHECK: andi $r0,  $r2,  7    ; encoding: [0x17,0x86,0x00,0x02]
ori   $r1,  $r5,  0   ;CHECK: ori  $r1,  $r5,  0    ; encoding: [0x68,0x88,0x00,0x02]
xori  $r6,  $r7,  6   ;CHECK: xori $r6,  $r7,  6    ; encoding: [0xbe,0x8b,0x00,0x02]
andi  $r8,  $r0,  123 ;CHECK: andi $r8,  $r0,  123  ; encoding: [0x03,0x86,0x47,0x06]
ori   $r5,  $r8,  12  ;CHECK: ori  $r5,  $r8,  12   ; encoding: [0x44,0x89,0x09,0x02]
xori  $r15, $r54, 15  ;CHECK: xori $r15, $r54, 15   ; encoding: [0xf7,0x8b,0x71,0x02]
andi  $r9,  $r14, 511 ;CHECK: andi $r9,  $r14, 511  ; encoding: [0x77,0x86,0x4f,0x1e]

; Logical operations with expressions
andi  $r0,  $r1,  (5 - 3)   ;CHECK: andi $r0,  $r1,  2  ; encoding: [0x0a,0x86,0x00,0x02]
xori  $r15, $r12, (0 - 0)   ;CHECK: xori $r15, $r12, 0  ; encoding: [0xe0,0x8b,0x48,0x02]
ori   $r43, $r19, (15 + 48) ;CHECK: ori  $r43, $r19, 63 ; encoding: [0xdf,0x88,0x57,0x03]

; Logical operations with relocations
andi  $r5,  $r1,  a   ;CHECK: andi $r5,  $r1, a   ; encoding: [0b01001AAA,0x87,0x00,0x02]
xori  $r11, $r5,  a+b ;CHECK: xori $r11, $r5, a+b ; encoding: [0b11101AAA,0x8a,0x40,0x02]
ori   $r0,  $r0,  c-a ;CHECK: ori  $r0,  $r0, c-a ; encoding: [0b00000AAA,0x88,0x00,0x02]


; Shift operations with immediates
lsli  $r0,  $r0,  1   ;CHECK: lsli $r0,  $r0,  1  ; encoding: [0x00,0x1a]
lsri  $r4,  $r0,  2   ;CHECK: lsri $r4,  $r0,  2  ; encoding: [0x01,0x1d]
asri  $r7,  $r1,  8   ;CHECK: asri $r7,  $r1,  8  ; encoding: [0xcf,0x19]
asri  $r7,  $r3,  5   ;CHECK: asri $r7,  $r3,  5  ; encoding: [0xdc,0x19]

lsli  $r0,  $r8,  1   ;CHECK: lsli $r0,  $r8,  1  ; encoding: [0x00,0x9a,0x08,0x00]
lsli  $r8,  $r5,  3   ;CHECK: lsli $r8,  $r5,  3  ; encoding: [0x2a,0x9a,0x40,0x00]
lsri  $r31, $r15, 4   ;CHECK: lsri $r31, $r15, 4  ; encoding: [0xfb,0x9d,0xc8,0x00]
asri  $r7,  $r35, 1   ;CHECK: asri $r7,  $r35, 1  ; encoding: [0xd8,0x99,0x20,0x00]

lsli  $r2,  $r4,  9   ;CHECK: lsli $r2,  $r4,  9  ; encoding: [0xa0,0x9a,0x01,0x00]
lsri  $r8,  $r0,  12  ;CHECK: lsri $r8,  $r0,  12 ; encoding: [0x03,0x9c,0x41,0x00]
asri  $r7,  $r1,  34  ;CHECK: asri $r7,  $r1,  34 ; encoding: [0xc9,0x99,0x04,0x00]
asri  $r7,  $r3,  63  ;CHECK: asri $r7,  $r3,  63 ; encoding: [0xde,0x99,0x07,0x00]

; Shift operations with expressions
lsri  $r4,  $r2,  (3 + 2)     ;CHECK: lsri $r4, $r2,  5 ; encoding: [0x14,0x1d]
asri  $r1,  $r1,  (1 + 0)     ;CHECK: asri $r1, $r1,  1 ; encoding: [0x48,0x18]
lsli  $r2,  $r7,  (128 - 65)  ;CHECK: lsli $r2, $r7, 63 ; encoding: [0xbe,0x9a,0x07,0x00]
lsri  $r8,  $r0,  (1 + 2)     ;CHECK: lsri $r8, $r0,  3 ; encoding: [0x02,0x9c,0x40,0x00]

; Shift operations with relocations
lsli  $r0,  $r5,  a   ;CHECK: lsli $r0, $r5, a ; encoding: [0b00101AAA,0x9a,0x00,0x00]
lsri  $r0,  $r5,  b   ;CHECK: lsri $r0, $r5, b ; encoding: [0b00101AAA,0x9c,0x00,0x00]
asri  $r0,  $r5,  c   ;CHECK: asri $r0, $r5, c ; encoding: [0b00101AAA,0x98,0x00,0x00]
