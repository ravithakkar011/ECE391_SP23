/* IDT.c - Manage and initialize interrupt descriptor table
 * vim:ts=4 noexpandtab
 * 
 * NOTES:
 *		- 5.1 ISA manual
 *      - enumerate a list of strings
 */

#include "IDT.h"

extern void syscall_handler_asm(void);

/*
  * Functions for each exception
  * DESCRIPTION: These are the handler functions for each exception. They print the error onto the screen.
  * INPUTS: none (void)
  * OUTPUS: none (void)
  * RETURN VALUE: none(void)
  * NOTES: Use printf and are sent through to SET_IDT_ENTRY
  *
  */
void Divide_Error(void){
    printf("Error: %s", "Divide_Error\n");
    // while(1);
    system_halt(255);
}

void RESERVED(void){
    printf("Error: %s","RESERVED\n");
    //while(1);
    system_halt(255);

}

void NMI_Interrupt(void){
    printf("Error: %s", "NMI_Interrupt\n");
    //while(1);
    system_halt(255);

}

void Breakpoint(void){
    printf("Error: %s", "Breakpoint\n");
    //while(1);
    system_halt(255);

}

void Overflow(void){
    printf("Error: %s",  "Overflow\n");
    // while(1);
    system_halt(255);

}

void BOUND_Range_Exceeded(void){
    printf("Error: %s", "BOUND_Range_Exceeded\n");
    // while(1);
    system_halt(255);

}

void Invalid_Opcode(void){
    printf("Error: %s","Invalid_Opcode\n");
    // while(1);
    system_halt(255);

}

void Device_Not_Available(void){
    printf("Error: %s", "Device_Not_Available\n");
    // while(1);
    system_halt(255);

}

void Double_Fault(void){
    printf("Error: %s", "Double_Fault\n");
    // while(1);
    system_halt(255);

}

void Coprocessor_Segment_Overrun(void){
    printf("Error: %s", "Coprocessor_Segment_Overrun\n");
    // while(1);
    system_halt(255);
}

void Invalid_TSS(void){
    printf("Error: %s", "Invalid_TSS\n");
    // while(1);
    system_halt(255);

}

void Segment_Not_Present(void){
    printf("Error: %s", "Segment_Not_Present\n");
    // while(1);
    system_halt(255);

}

void Stack_Segment_Fault(void){
    printf("Error: %s", "Stack_Segment_Fault\n");
    // while(1);
    system_halt(255);

}

void General_Protection(void){
    printf("Error: %s", "General_Protection\n");
    // while(1);
    system_halt(255);

}

void Page_Fault(void){
    printf("Error: %s", "Page_Fault\n");
    // while(1);
    system_halt(255);

}

void Intel_Reserved(void){
    printf("Error: %s", "Intel_Reserved\n");
    // while(1);
    system_halt(255);

}

void x87_FPU_Floating_Point_Error(void){
    printf("Error: %s", "x87_FPU_Floating_Point_Error\n");
    // while(1);
    system_halt(255);

}

void Alignment_Check(void){
    printf("Error: %s", "Alignment_Check\n");
    // while(1);
    system_halt(255);

}

void Machine_Check(void){
    printf("Error: %s", "Machine_Check\n");
    // while(1);
    system_halt(255);

}

void SIMD_Floating_Point_Exception(void){
    printf("Error: %s", "SIMD_Floating_Point_Exception\n");
    // while(1);
    system_halt(255);
}

// void syscall_handler(void){
//     syscall_handler_asm();
// }

/*
  * init_idt
  * DESCRIPTION: initializes IDT
  * INPUTS: none (void)
  * OUTPUS: none (void)
  * RETURN VALUE: none(void)
  * NOTES: idt struct values initialized based on osdev directions
  *
  */
void init_IDT(void){

    int i;
    for (i = 0; i < VECTOR_NUM; i++){ //iterates through every interrupt code
        
        idt[i].present = 0; //initially sets present to 0 for each vector
        if (i < EXCEPTIONS_NUM && i != 0x15){ // if the vector is an exception that we need to take care of
            idt[i].present = 1; // turn present high for that vector
            idt[i].dpl = 0;
        }

        if (i == SYSCALL){ // check if i is a system call
            idt[i].dpl = 3; 
            idt[i].present = 1; //turn present high and dpl to 3 for syscalls
        }


        // set signals based on osdev
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0; 
        idt[i].size = 1;
       
        
    }
    //calls SET_IDT_ENTRY for each exception
    SET_IDT_ENTRY(idt[0], Divide_Error_link);

    SET_IDT_ENTRY(idt[1], RESERVED_link);

    SET_IDT_ENTRY(idt[2], NMI_Interrupt_link);

    SET_IDT_ENTRY(idt[3], Breakpoint_link);

    SET_IDT_ENTRY(idt[4], Overflow_link);

    SET_IDT_ENTRY(idt[5], BOUND_Range_Exceeded_link);

    SET_IDT_ENTRY(idt[6], Invalid_Opcode_link);

    SET_IDT_ENTRY(idt[7], Device_Not_Available_link);

    SET_IDT_ENTRY(idt[8], Double_Fault_link);

    SET_IDT_ENTRY(idt[9], Coprocessor_Segment_Overrun_link);

    SET_IDT_ENTRY(idt[10], Invalid_TSS_link);

    SET_IDT_ENTRY(idt[11], Segment_Not_Present_link);

    SET_IDT_ENTRY(idt[12], Stack_Segment_Fault_link);

    SET_IDT_ENTRY(idt[13], General_Protection_link);

    SET_IDT_ENTRY(idt[14], Page_Fault_link);

    SET_IDT_ENTRY(idt[15], Intel_Reserved_link);

    SET_IDT_ENTRY(idt[16], x87_FPU_Floating_Point_Error_link);

    SET_IDT_ENTRY(idt[17], Alignment_Check_link);

    SET_IDT_ENTRY(idt[18], Machine_Check_link);

    SET_IDT_ENTRY(idt[19], SIMD_Floating_Point_Exception_link);

    SET_IDT_ENTRY(idt[20], Intel_Reserved_link);


    /*Initializing Gate Descriptors for devices*/
    // keyboard
    idt[KEYBOARD_IRQ].present = 1; // set present state high
    idt[KEYBOARD_IRQ].dpl = 0;
    //idt[KEYBOARD_IRQ].reserved3 = 0;
    SET_IDT_ENTRY(idt[KEYBOARD_IRQ], keyboard_handler_link);
    
    
    // rtc
    idt[RTC_IRQ].present = 1;
    idt[RTC_IRQ].dpl = 0;
    SET_IDT_ENTRY(idt[RTC_IRQ], rtc_handler_link);
    
    
    /*Setting IDT Entry for Syscall*/
    
    SET_IDT_ENTRY(idt[SYSCALL], syscall_handler);

}
