; RUN: llvm-mc %s -filetype=obj -triple=aap | llvm-readobj -h \
; RUN:     | FileCheck %s

; CHECK: Format: ELF32-aap
; CHECK: Arch: aap
; CHECK: AddressSize: 32bit
; CHECK: ElfHeader {
; CHECK:   Ident {
; CHECK:     Magic: (7F 45 4C 46)
; CHECK:     Class: 32-bit (0x1)
; CHECK:     DataEncoding: LittleEndian (0x1)
; CHECK:     FileVersion: 1
; CHECK:     OS/ABI: SystemV (0x0)
; CHECK:     ABIVersion: 0
; CHECK:   }
; CHECK:   Type: Relocatable (0x1)
; CHECK:   Machine: EM_AAP (0x5343)
; CHECK:   Version: 1
; CHECK:   Flags [ (0x0)
; CHECK:   ]
; CHECK: }
