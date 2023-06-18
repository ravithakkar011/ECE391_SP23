/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = MS_MASK; /* IRQs 0-7  */
uint8_t slave_mask = MS_MASK;  /* IRQs 8-15 */

/*
 * i8259_init
 * DESCRIPTION: Initialize the 8259 PIC (master and slave interrupt ctlers
 * INPUTS: none
 * OUTPUS: none
 * RETURN VALUE: none
 */
void i8259_init(void) {
    // master_mask = 0xff;
	// slave_mask = 0xff;
    // spinlock?

    outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);

	/* send 0x11 for init mode in case of no grub*/
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);
    /* master & slave offset */
    outb(ICW2_MASTER, MASTER_DATA);                                 // mapped to 0-7 irqs (port + 0)
	outb(ICW2_SLAVE, SLAVE_DATA);                                   // mapped to 8-15 irqs (port + 8)
    /* start pic in cascade mode (pics connected in series) */
	outb(ICW3_MASTER, MASTER_DATA);
	outb(ICW2_SLAVE, SLAVE_DATA);

	/* tell master there is a slave & tell slave about master*/
	outb(ICW4, MASTER_DATA);
	outb(ICW4, SLAVE_DATA);

    outb(master_mask, MASTER_DATA);
    outb(slave_mask, SLAVE_DATA);

	/* enable irq */
	enable_irq(NUM_PIC);

}

/*
 * enable_irq
 * DESCRIPTION: Enable (unmask) the specified IRQ
 * INPUTS: irq_num - interrupt ID number
 * OUTPUS: none
 * RETURN VALUE: none
 */
void enable_irq(uint32_t irq_num) {
    uint32_t offset = irq_num - IRQ_PRIM_NUM;

    if (irq_num < IRQ_PRIM_NUM) {                                          // primary IC (8 <= IRQ <= 15)
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, MASTER_DATA);
    }
    else if (IRQ_PRIM_NUM <= irq_num && irq_num <= IRQ_NUM) {               // secondary IC
        slave_mask = slave_mask & ~(1 << offset);
        outb(slave_mask, SLAVE_DATA);
    }
    else{
        return;
    }
}

/*
 * disable_irq
 * DESCRIPTION: Disable (mask) the specified IRQ
 * INPUTS: irq_num - interrupt ID number
 * OUTPUS: none
 * RETURN VALUE: none
 */
void disable_irq(uint32_t irq_num) {
    uint32_t offset = irq_num - IRQ_PRIM_NUM;

    // if(irq_num > IRQ_NUM){
    //     return;
    // }
    if (irq_num < IRQ_PRIM_NUM) {                                             // primary IC (8 <= IRQ <= 15)
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, MASTER_DATA);
    }
    else if(IRQ_PRIM_NUM <= irq_num && irq_num <= IRQ_NUM) {                   // secondary IC
        slave_mask = slave_mask | (1 << offset);
        outb(slave_mask, SLAVE_DATA);
    }
    else{
        return;
    }
}

/* 
 * send_eoi
 * DESCRIPTION: Send end-of-interrupt signal for the specified IRQ 
 * INPUTS: irq_num - interrupt ID number
 * OUTPUS:
 * RETURN VALUE: none
 */
void send_eoi(uint32_t irq_num) {
    if (irq_num < IRQ_PRIM_NUM) {                                  // primary IC
		outb(irq_num | EOI, MASTER_8259_PORT);
	}
    else if (IRQ_PRIM_NUM <= irq_num && irq_num <= IRQ_NUM) {       // secondary IC; send both
        outb((irq_num - IRQ_PRIM_NUM) | EOI, SLAVE_8259_PORT);      // subtract by offset of 8 to get to secondary IC
        outb(IRQ_SECONDARY | EOI, MASTER_8259_PORT);
    }
    else {
        return;
    }
}
