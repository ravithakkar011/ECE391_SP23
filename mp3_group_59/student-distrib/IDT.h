/* IDT.h - Headers & defines used to manage and initialize interrupt descriptor table
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_H
#define _IDT_H

#ifndef ASM

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
//#include "systemcall_handler.h"
#include "syscall.h"
#include "assembly_linkage.h"


#define VECTOR_NUM      256         // or number of descriptors
#define SYSCALL         0x80
#define IRQ_MAPPED      0x20
#define EXCEPTIONS_NUM  21
#define KEYBOARD_IRQ    0x21        // IDT table index
#define RTC_IRQ         0x28


extern void Divide_Error(void);

extern void RESERVED(void);

extern void NMI_Interrupt(void);

extern void Breakpoint(void);

extern void Overflow(void);

extern void BOUND_Range_Exceeded(void);

extern void Invalid_Opcode(void);

extern void Device_Not_Available(void);

extern void Double_Fault(void);

extern void Coprocessor_Segment_Overrun(void);

extern void Invalid_TSS(void);

extern void Segment_Not_Present(void);

extern void Stack_Segment_Fault(void);

extern void General_Protection(void);

extern void Page_Fault(void);

extern void Intel_Reserved(void);

extern void x87_FPU_Floating_Point_Error(void);

extern void Alignment_Check(void);

extern void Machine_Check(void);

extern void SIMD_Floating_Point_Exception(void);

//extern void syscall_handler(void);

/* Initialize Interrupt Descriptor Table */
extern void init_IDT(void);


#endif
#endif
