; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown-unknown | FileCheck %s --check-prefixes=X86,X86-NOBMI
; RUN: llc < %s -mtriple=i686-unknown-unknown -mattr=+bmi2 | FileCheck %s --check-prefixes=X86,X86-BMI
; RUN: llc < %s -mtriple=x86_64-unknown-unknown | FileCheck %s --check-prefixes=X64,X64-NOBMI
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+bmi2 | FileCheck %s --check-prefixes=X64,X64-BMI

; PR1198

define i64 @foo(i64 %x, i64 %y) nounwind {
; X86-NOBMI-LABEL: foo:
; X86-NOBMI:       # %bb.0:
; X86-NOBMI-NEXT:    pushl %ebp
; X86-NOBMI-NEXT:    pushl %ebx
; X86-NOBMI-NEXT:    pushl %edi
; X86-NOBMI-NEXT:    pushl %esi
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ebp
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %edi
; X86-NOBMI-NEXT:    movl %ecx, %eax
; X86-NOBMI-NEXT:    mull %ebp
; X86-NOBMI-NEXT:    movl %edx, %ebx
; X86-NOBMI-NEXT:    movl %esi, %eax
; X86-NOBMI-NEXT:    mull %ebp
; X86-NOBMI-NEXT:    movl %edx, %ebp
; X86-NOBMI-NEXT:    movl %eax, %esi
; X86-NOBMI-NEXT:    addl %ebx, %esi
; X86-NOBMI-NEXT:    adcl $0, %ebp
; X86-NOBMI-NEXT:    movl %ecx, %eax
; X86-NOBMI-NEXT:    mull %edi
; X86-NOBMI-NEXT:    movl %edx, %ebx
; X86-NOBMI-NEXT:    addl %esi, %eax
; X86-NOBMI-NEXT:    adcl %ebp, %ebx
; X86-NOBMI-NEXT:    setb %al
; X86-NOBMI-NEXT:    movzbl %al, %ecx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    mull %edi
; X86-NOBMI-NEXT:    movl %edx, %esi
; X86-NOBMI-NEXT:    movl %eax, %ebp
; X86-NOBMI-NEXT:    addl %ebx, %ebp
; X86-NOBMI-NEXT:    adcl %ecx, %esi
; X86-NOBMI-NEXT:    xorl %ecx, %ecx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    mull %ecx
; X86-NOBMI-NEXT:    movl %edx, %edi
; X86-NOBMI-NEXT:    movl %eax, %ebx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    mull %ecx
; X86-NOBMI-NEXT:    addl %ebx, %eax
; X86-NOBMI-NEXT:    adcl %edi, %edx
; X86-NOBMI-NEXT:    addl %ebp, %eax
; X86-NOBMI-NEXT:    adcl %esi, %edx
; X86-NOBMI-NEXT:    popl %esi
; X86-NOBMI-NEXT:    popl %edi
; X86-NOBMI-NEXT:    popl %ebx
; X86-NOBMI-NEXT:    popl %ebp
; X86-NOBMI-NEXT:    retl
;
; X86-BMI-LABEL: foo:
; X86-BMI:       # %bb.0:
; X86-BMI-NEXT:    pushl %ebp
; X86-BMI-NEXT:    pushl %ebx
; X86-BMI-NEXT:    pushl %edi
; X86-BMI-NEXT:    pushl %esi
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %edi
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    mulxl %esi, %edx, %ebx
; X86-BMI-NEXT:    movl %eax, %edx
; X86-BMI-NEXT:    mulxl %esi, %ebp, %eax
; X86-BMI-NEXT:    addl %ebx, %ebp
; X86-BMI-NEXT:    adcl $0, %eax
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    mulxl %edi, %edx, %ebx
; X86-BMI-NEXT:    addl %ebp, %edx
; X86-BMI-NEXT:    adcl %eax, %ebx
; X86-BMI-NEXT:    setb %al
; X86-BMI-NEXT:    movzbl %al, %eax
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X86-BMI-NEXT:    mulxl %edi, %edi, %ebp
; X86-BMI-NEXT:    addl %ebx, %edi
; X86-BMI-NEXT:    adcl %eax, %ebp
; X86-BMI-NEXT:    xorl %eax, %eax
; X86-BMI-NEXT:    movl %esi, %edx
; X86-BMI-NEXT:    mulxl %eax, %ebx, %esi
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    mulxl %eax, %eax, %edx
; X86-BMI-NEXT:    addl %ebx, %eax
; X86-BMI-NEXT:    adcl %esi, %edx
; X86-BMI-NEXT:    addl %edi, %eax
; X86-BMI-NEXT:    adcl %ebp, %edx
; X86-BMI-NEXT:    popl %esi
; X86-BMI-NEXT:    popl %edi
; X86-BMI-NEXT:    popl %ebx
; X86-BMI-NEXT:    popl %ebp
; X86-BMI-NEXT:    retl
;
; X64-NOBMI-LABEL: foo:
; X64-NOBMI:       # %bb.0:
; X64-NOBMI-NEXT:    movq %rdi, %rax
; X64-NOBMI-NEXT:    mulq %rsi
; X64-NOBMI-NEXT:    movq %rdx, %rax
; X64-NOBMI-NEXT:    retq
;
; X64-BMI-LABEL: foo:
; X64-BMI:       # %bb.0:
; X64-BMI-NEXT:    movq %rdi, %rdx
; X64-BMI-NEXT:    mulxq %rsi, %rcx, %rax
; X64-BMI-NEXT:    retq
  %tmp0 = zext i64 %x to i128
  %tmp1 = zext i64 %y to i128
  %tmp2 = mul i128 %tmp0, %tmp1
  %tmp7 = zext i32 64 to i128
  %tmp3 = lshr i128 %tmp2, %tmp7
  %tmp4 = trunc i128 %tmp3 to i64
  ret i64 %tmp4
}

; <rdar://problem/14096009> superfluous multiply by high part of
; zero-extended value.

define i64 @mul1(i64 %n, i64* nocapture %z, i64* nocapture %x, i64 %y) nounwind {
; X86-NOBMI-LABEL: mul1:
; X86-NOBMI:       # %bb.0: # %entry
; X86-NOBMI-NEXT:    pushl %ebp
; X86-NOBMI-NEXT:    pushl %ebx
; X86-NOBMI-NEXT:    pushl %edi
; X86-NOBMI-NEXT:    pushl %esi
; X86-NOBMI-NEXT:    subl $28, %esp
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    orl %ecx, %eax
; X86-NOBMI-NEXT:    je .LBB1_3
; X86-NOBMI-NEXT:  # %bb.1: # %for.body.preheader
; X86-NOBMI-NEXT:    xorl %eax, %eax
; X86-NOBMI-NEXT:    xorl %edx, %edx
; X86-NOBMI-NEXT:    xorl %ebx, %ebx
; X86-NOBMI-NEXT:    movl $0, {{[0-9]+}}(%esp) # 4-byte Folded Spill
; X86-NOBMI-NEXT:    .p2align 4, 0x90
; X86-NOBMI-NEXT:  .LBB1_2: # %for.body
; X86-NOBMI-NEXT:    # =>This Inner Loop Header: Depth=1
; X86-NOBMI-NEXT:    movl %edx, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl %eax, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    movl %eax, %ecx
; X86-NOBMI-NEXT:    movl (%eax,%ebx,8), %ebp
; X86-NOBMI-NEXT:    movl 4(%eax,%ebx,8), %esi
; X86-NOBMI-NEXT:    movl %esi, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl %ebp, %eax
; X86-NOBMI-NEXT:    movl %ebp, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NOBMI-NEXT:    mull %ecx
; X86-NOBMI-NEXT:    movl %edx, %edi
; X86-NOBMI-NEXT:    movl %eax, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl %esi, %eax
; X86-NOBMI-NEXT:    mull %ecx
; X86-NOBMI-NEXT:    movl %edx, %ecx
; X86-NOBMI-NEXT:    movl %eax, %esi
; X86-NOBMI-NEXT:    addl %edi, %esi
; X86-NOBMI-NEXT:    adcl $0, %ecx
; X86-NOBMI-NEXT:    movl %ebp, %eax
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X86-NOBMI-NEXT:    mull %edx
; X86-NOBMI-NEXT:    movl %edx, %ebp
; X86-NOBMI-NEXT:    movl %eax, %edi
; X86-NOBMI-NEXT:    addl %esi, %edi
; X86-NOBMI-NEXT:    adcl %ecx, %ebp
; X86-NOBMI-NEXT:    setb {{[0-9]+}}(%esp) # 1-byte Folded Spill
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax # 4-byte Reload
; X86-NOBMI-NEXT:    mull {{[0-9]+}}(%esp)
; X86-NOBMI-NEXT:    movl %edx, %ecx
; X86-NOBMI-NEXT:    movl %eax, %esi
; X86-NOBMI-NEXT:    addl %ebp, %esi
; X86-NOBMI-NEXT:    movzbl {{[0-9]+}}(%esp), %eax # 1-byte Folded Reload
; X86-NOBMI-NEXT:    adcl %eax, %ecx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-NOBMI-NEXT:    xorl %edx, %edx
; X86-NOBMI-NEXT:    mull %edx
; X86-NOBMI-NEXT:    movl %edx, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    movl %eax, %ebp
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %eax # 4-byte Reload
; X86-NOBMI-NEXT:    xorl %edx, %edx
; X86-NOBMI-NEXT:    mull %edx
; X86-NOBMI-NEXT:    addl %ebp, %eax
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ebp
; X86-NOBMI-NEXT:    adcl {{[0-9]+}}(%esp), %edx # 4-byte Folded Reload
; X86-NOBMI-NEXT:    addl %esi, %eax
; X86-NOBMI-NEXT:    adcl %ecx, %edx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %esi # 4-byte Reload
; X86-NOBMI-NEXT:    addl {{[0-9]+}}(%esp), %esi # 4-byte Folded Reload
; X86-NOBMI-NEXT:    adcl {{[0-9]+}}(%esp), %edi # 4-byte Folded Reload
; X86-NOBMI-NEXT:    adcl $0, %eax
; X86-NOBMI-NEXT:    adcl $0, %edx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NOBMI-NEXT:    movl %esi, (%ecx,%ebx,8)
; X86-NOBMI-NEXT:    movl %edi, 4(%ecx,%ebx,8)
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-NOBMI-NEXT:    movl %ecx, %edi
; X86-NOBMI-NEXT:    addl $1, %ebx
; X86-NOBMI-NEXT:    movl {{[0-9]+}}(%esp), %esi # 4-byte Reload
; X86-NOBMI-NEXT:    adcl $0, %esi
; X86-NOBMI-NEXT:    movl %ebx, %ecx
; X86-NOBMI-NEXT:    xorl %ebp, %ecx
; X86-NOBMI-NEXT:    movl %esi, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-NOBMI-NEXT:    xorl %edi, %esi
; X86-NOBMI-NEXT:    orl %ecx, %esi
; X86-NOBMI-NEXT:    jne .LBB1_2
; X86-NOBMI-NEXT:  .LBB1_3: # %for.end
; X86-NOBMI-NEXT:    xorl %eax, %eax
; X86-NOBMI-NEXT:    xorl %edx, %edx
; X86-NOBMI-NEXT:    addl $28, %esp
; X86-NOBMI-NEXT:    popl %esi
; X86-NOBMI-NEXT:    popl %edi
; X86-NOBMI-NEXT:    popl %ebx
; X86-NOBMI-NEXT:    popl %ebp
; X86-NOBMI-NEXT:    retl
;
; X86-BMI-LABEL: mul1:
; X86-BMI:       # %bb.0: # %entry
; X86-BMI-NEXT:    pushl %ebp
; X86-BMI-NEXT:    pushl %ebx
; X86-BMI-NEXT:    pushl %edi
; X86-BMI-NEXT:    pushl %esi
; X86-BMI-NEXT:    subl $20, %esp
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    orl %ecx, %eax
; X86-BMI-NEXT:    je .LBB1_3
; X86-BMI-NEXT:  # %bb.1: # %for.body.preheader
; X86-BMI-NEXT:    xorl %ecx, %ecx
; X86-BMI-NEXT:    xorl %edx, %edx
; X86-BMI-NEXT:    xorl %edi, %edi
; X86-BMI-NEXT:    movl $0, {{[0-9]+}}(%esp) # 4-byte Folded Spill
; X86-BMI-NEXT:    .p2align 4, 0x90
; X86-BMI-NEXT:  .LBB1_2: # %for.body
; X86-BMI-NEXT:    # =>This Inner Loop Header: Depth=1
; X86-BMI-NEXT:    movl %ecx, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl %edx, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    movl (%eax,%edi,8), %ecx
; X86-BMI-NEXT:    movl 4(%eax,%edi,8), %ebx
; X86-BMI-NEXT:    movl %ebx, (%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    movl %eax, %esi
; X86-BMI-NEXT:    mulxl %eax, %eax, %ebp
; X86-BMI-NEXT:    movl %eax, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl %ebx, %edx
; X86-BMI-NEXT:    mulxl %esi, %eax, %esi
; X86-BMI-NEXT:    addl %ebp, %eax
; X86-BMI-NEXT:    adcl $0, %esi
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    mulxl {{[0-9]+}}(%esp), %ebp, %ebx
; X86-BMI-NEXT:    addl %eax, %ebp
; X86-BMI-NEXT:    adcl %esi, %ebx
; X86-BMI-NEXT:    movl (%esp), %edx # 4-byte Reload
; X86-BMI-NEXT:    mulxl {{[0-9]+}}(%esp), %eax, %esi
; X86-BMI-NEXT:    setb %dl
; X86-BMI-NEXT:    addl %ebx, %eax
; X86-BMI-NEXT:    movzbl %dl, %edx
; X86-BMI-NEXT:    adcl %edx, %esi
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X86-BMI-NEXT:    xorl %ebx, %ebx
; X86-BMI-NEXT:    mulxl %ebx, %ebx, %edx
; X86-BMI-NEXT:    movl %edx, (%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl %ecx, %edx
; X86-BMI-NEXT:    xorl %ecx, %ecx
; X86-BMI-NEXT:    mulxl %ecx, %ecx, %edx
; X86-BMI-NEXT:    addl %ebx, %ecx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %ebx
; X86-BMI-NEXT:    adcl (%esp), %edx # 4-byte Folded Reload
; X86-BMI-NEXT:    addl %eax, %ecx
; X86-BMI-NEXT:    adcl %esi, %edx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %esi # 4-byte Reload
; X86-BMI-NEXT:    addl {{[0-9]+}}(%esp), %esi # 4-byte Folded Reload
; X86-BMI-NEXT:    adcl {{[0-9]+}}(%esp), %ebp # 4-byte Folded Reload
; X86-BMI-NEXT:    adcl $0, %ecx
; X86-BMI-NEXT:    adcl $0, %edx
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    movl %esi, (%eax,%edi,8)
; X86-BMI-NEXT:    movl %ebp, 4(%eax,%edi,8)
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X86-BMI-NEXT:    movl %eax, %esi
; X86-BMI-NEXT:    addl $1, %edi
; X86-BMI-NEXT:    movl {{[0-9]+}}(%esp), %ebp # 4-byte Reload
; X86-BMI-NEXT:    adcl $0, %ebp
; X86-BMI-NEXT:    movl %edi, %eax
; X86-BMI-NEXT:    xorl %esi, %eax
; X86-BMI-NEXT:    movl %ebp, {{[0-9]+}}(%esp) # 4-byte Spill
; X86-BMI-NEXT:    movl %ebp, %esi
; X86-BMI-NEXT:    xorl %ebx, %esi
; X86-BMI-NEXT:    orl %eax, %esi
; X86-BMI-NEXT:    jne .LBB1_2
; X86-BMI-NEXT:  .LBB1_3: # %for.end
; X86-BMI-NEXT:    xorl %eax, %eax
; X86-BMI-NEXT:    xorl %edx, %edx
; X86-BMI-NEXT:    addl $20, %esp
; X86-BMI-NEXT:    popl %esi
; X86-BMI-NEXT:    popl %edi
; X86-BMI-NEXT:    popl %ebx
; X86-BMI-NEXT:    popl %ebp
; X86-BMI-NEXT:    retl
;
; X64-NOBMI-LABEL: mul1:
; X64-NOBMI:       # %bb.0: # %entry
; X64-NOBMI-NEXT:    testq %rdi, %rdi
; X64-NOBMI-NEXT:    je .LBB1_3
; X64-NOBMI-NEXT:  # %bb.1: # %for.body.preheader
; X64-NOBMI-NEXT:    movq %rcx, %r8
; X64-NOBMI-NEXT:    movq %rdx, %r9
; X64-NOBMI-NEXT:    xorl %r10d, %r10d
; X64-NOBMI-NEXT:    xorl %ecx, %ecx
; X64-NOBMI-NEXT:    .p2align 4, 0x90
; X64-NOBMI-NEXT:  .LBB1_2: # %for.body
; X64-NOBMI-NEXT:    # =>This Inner Loop Header: Depth=1
; X64-NOBMI-NEXT:    movq %r8, %rax
; X64-NOBMI-NEXT:    mulq (%r9,%rcx,8)
; X64-NOBMI-NEXT:    addq %r10, %rax
; X64-NOBMI-NEXT:    adcq $0, %rdx
; X64-NOBMI-NEXT:    movq %rax, (%rsi,%rcx,8)
; X64-NOBMI-NEXT:    incq %rcx
; X64-NOBMI-NEXT:    cmpq %rcx, %rdi
; X64-NOBMI-NEXT:    movq %rdx, %r10
; X64-NOBMI-NEXT:    jne .LBB1_2
; X64-NOBMI-NEXT:  .LBB1_3: # %for.end
; X64-NOBMI-NEXT:    xorl %eax, %eax
; X64-NOBMI-NEXT:    retq
;
; X64-BMI-LABEL: mul1:
; X64-BMI:       # %bb.0: # %entry
; X64-BMI-NEXT:    testq %rdi, %rdi
; X64-BMI-NEXT:    je .LBB1_3
; X64-BMI-NEXT:  # %bb.1: # %for.body.preheader
; X64-BMI-NEXT:    movq %rcx, %r8
; X64-BMI-NEXT:    movq %rdx, %r9
; X64-BMI-NEXT:    xorl %r10d, %r10d
; X64-BMI-NEXT:    xorl %eax, %eax
; X64-BMI-NEXT:    .p2align 4, 0x90
; X64-BMI-NEXT:  .LBB1_2: # %for.body
; X64-BMI-NEXT:    # =>This Inner Loop Header: Depth=1
; X64-BMI-NEXT:    movq %r8, %rdx
; X64-BMI-NEXT:    mulxq (%r9,%rax,8), %rcx, %rdx
; X64-BMI-NEXT:    addq %r10, %rcx
; X64-BMI-NEXT:    adcq $0, %rdx
; X64-BMI-NEXT:    movq %rcx, (%rsi,%rax,8)
; X64-BMI-NEXT:    incq %rax
; X64-BMI-NEXT:    cmpq %rax, %rdi
; X64-BMI-NEXT:    movq %rdx, %r10
; X64-BMI-NEXT:    jne .LBB1_2
; X64-BMI-NEXT:  .LBB1_3: # %for.end
; X64-BMI-NEXT:    xorl %eax, %eax
; X64-BMI-NEXT:    retq
entry:
  %conv = zext i64 %y to i128
  %cmp11 = icmp eq i64 %n, 0
  br i1 %cmp11, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %carry.013 = phi i64 [ %conv6, %for.body ], [ 0, %entry ]
  %i.012 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i64, i64* %x, i64 %i.012
  %0 = load i64, i64* %arrayidx, align 8
  %conv2 = zext i64 %0 to i128
  %mul = mul i128 %conv2, %conv
  %conv3 = zext i64 %carry.013 to i128
  %add = add i128 %mul, %conv3
  %conv4 = trunc i128 %add to i64
  %arrayidx5 = getelementptr inbounds i64, i64* %z, i64 %i.012
  store i64 %conv4, i64* %arrayidx5, align 8
  %shr = lshr i128 %add, 64
  %conv6 = trunc i128 %shr to i64
  %inc = add i64 %i.012, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret i64 0
}
