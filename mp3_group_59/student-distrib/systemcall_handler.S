#define ASM     1
#define IRQ_SYSCALL 0x80

.globl syscall_handler ;\
syscall_handler:

    pushl %ebp  #saves the registers to the stack
    pushl %edi
    pushl %esi

    pushfl    #pushes the flags to the stack

 
    pushl %edx # pushes the parameters to the stack (right to left)
    pushl %ecx
    pushl %ebx

    # see if curr system call cmd is within range
    cmpl $0, %eax # index < 0?
    jle command_invalid

    cmpl $10, %eax # index > 10?
    jg command_invalid


    call *system_table(, %eax, 4)     #calls jump table
    jmp teardown

command_invalid:
    movl $-1, %eax

teardown:
    #pop parameters
    addl $12, %esp

    popfl
    popl %esi
    popl %edi
    popl %ebp
    iret

system_table:
    .long 0x00000000, system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap

