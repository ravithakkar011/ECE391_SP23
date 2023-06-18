#define ASM     1
#define IRQ_Keyboard    0x21
#define IRQ_RTC     0x28
#define IRQ_SYSCALL 0x80
.text
#define LINK(name, handler, irq)  \
    .globl name         ;\
    name:               ;\
        pushal          ;\
        pushfl          ;\
        pushl $irq      ;\
        call handler     ;\
        popl %eax       ;\
        popfl           ;\
        popal           ;\
        iret

LINK(Divide_Error_link, Divide_Error, 0);
LINK(RESERVED_link, RESERVED, 1);
LINK(NMI_Interrupt_link, NMI_Interrupt,2);
LINK(Breakpoint_link, Breakpoint,3);
LINK(Overflow_link, Overflow, 4);
LINK(BOUND_Range_Exceeded_link, BOUND_Range_Exceeded, 5);
LINK(Invalid_Opcode_link, Invalid_Opcode, 6);
LINK(Device_Not_Available_link, Device_Not_Available, 7);
LINK(Double_Fault_link, Double_Fault, 8);
LINK(Coprocessor_Segment_Overrun_link, Coprocessor_Segment_Overrun, 9);
LINK(Invalid_TSS_link, Invalid_TSS, 10);
LINK(Segment_Not_Present_link, Segment_Not_Present, 11);
LINK(Stack_Segment_Fault_link, Stack_Segment_Fault, 12);
LINK(General_Protection_link, General_Protection, 13);
LINK(Page_Fault_link, Page_Fault, 14);
LINK(Intel_Reserved_link, Intel_Reserved, 15);
LINK(x87_FPU_Floating_Point_Error_link, x87_FPU_Floating_Point_Error, 16);
LINK(Alignment_Check_link, Alignment_Check, 17);
LINK(Machine_Check_link, Machine_Check, 18);
LINK(SIMD_Floating_Point_Exception_link, SIMD_Floating_Point_Exception, 19);

LINK(keyboard_handler_link, keyboard_handler, IRQ_Keyboard);
LINK(rtc_handler_link, rtc_handler, IRQ_RTC);
