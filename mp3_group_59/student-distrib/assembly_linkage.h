/* assembly_linkage.h - help link assembly functions for IDT.c
 * vim:ts=4 noexpandtab
 */

#ifndef _ASM_H
#define _ASM_H
//#define ASM      

//#include "x86_desc.h"
//#include "IDT.h"
//#include "lib.h"

/* exceptions linkage */
 void Divide_Error_link();
 void RESERVED_link();
 void NMI_Interrupt_link();
 void Breakpoint_link();
 void Overflow_link();
 void BOUND_Range_Exceeded_link();
 void Invalid_Opcode_link();
 void Device_Not_Available_link();
 void Double_Fault_link();
 void Coprocessor_Segment_Overrun_link();
 void Invalid_TSS_link();
 void Segment_Not_Present_link();
 void Stack_Segment_Fault_link();
 void General_Protection_link();
 void Page_Fault_link();
 void Intel_Reserved_link();
 void x87_FPU_Floating_Point_Error_link();
 void Alignment_Check_link();
 void Machine_Check_link();
 void SIMD_Floating_Point_Exception_link();

/* keyboard & rtc linkage */
 void rtc_handler_link();
 void keyboard_handler_link();
 
//  /*systemcall linkage */
//  extern void syscall_handler();
// // void syscall_handler_link();

#endif
