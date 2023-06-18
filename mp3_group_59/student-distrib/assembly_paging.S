#define ASM     1

.globl enable
.align 4
enable:
  pushl %ebp 
  movl  %esp, %ebp                  # set up stack
  movl  8(%ebp), %eax
  movl %eax, %cr3
  movl %cr4, %eax
  orl  $0x00000010, %eax            # enable PSE (32-bit paging)
  movl %eax, %cr4
  movl %cr0, %eax
  orl  $0x80000001, %eax            # read & write 
  movl %eax, %cr0
  movl %cr3, %eax
  movl %eax, %cr3

  leave
  ret


.globl flush_tlb
.align 4
flush_tlb:
    movl %cr3, %eax    
    movl %eax, %cr3
    ret
