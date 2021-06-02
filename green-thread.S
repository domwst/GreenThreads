    .att_syntax
    .text
.global GreenThreadSwitch
GreenThreadSwitch:
    movq %rbx,   (%rdi)
    movq %rsp,  8(%rdi)
    movq %rbp, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

    movq   (%rsi), %rbx
    movq  8(%rsi), %rsp
    movq 16(%rsi), %rbp
    movq 24(%rsi), %r12
    movq 32(%rsi), %r13
    movq 40(%rsi), %r14
    movq 48(%rsi), %r15
    ret

.global GreenThreadFirstSwitch
GreenThreadFirstSwitch:
    movq %rbx,   (%rdi)
    movq %rsp,  8(%rdi)
    movq %rbp, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

    movq 56(%rsi), %rsp# topOfStack --> rsp

    movq %rsi, %rdi
    xorq %rsi, %rsi

    call GreenThreadExecute