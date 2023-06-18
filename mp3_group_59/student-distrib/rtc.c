#define RTC_IRQ 0x08
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define RTC_REGA 0x8A
#define RTC_REGB 0x8B
#define RTC_REGC 0x0C
#define SECONDARY_PIC_IRQ 0x02
#define TOP_4_BITS 0xF0
#include "rtc.h"
volatile uint32_t in_count;
/* void rtc_init();
 * Inputs: void
 * Return Value: none
 * Function: Initializes RTC */

void rtc_init(){
    cli(); //clear interrupt flag (must use sti now at the end)
    //disable_irq(RTC_IRQ); // disables interrupts
    outb(RTC_REGB, RTC_PORT); // selects register B and disables NMI
    char prev = inb(CMOS_PORT);  // reads value of register B
    outb(RTC_REGB,RTC_PORT); // re-sets index to register B
    outb(prev | 0x40,CMOS_PORT); // ORs the previous value with 0x40 so it turns on bit 6 of register B
    enable_irq(RTC_IRQ); //enables interrupts
    enable_irq(SECONDARY_PIC_IRQ); //enable interrupts for the secondary PIC irq just in case
    sti(); //reset interrupt flag because we used cli();

    //rtc defaults to 1024 hz
}

void rtc_handler(){
    cli(); //clear interrupt flag (must use sti now at the end)
    //test_interrupts();

    outb(RTC_REGC, RTC_PORT); //selects register C
    inb(CMOS_PORT); // throws away contents
    // counter counts up
    in_count = 1;

    send_eoi(RTC_IRQ); //sends end-of-line interrupt
    
    sti(); //reset interrupt flag because we used cli();
}

/*
 * rtc_set_freq
 * calculate the frequency index
 *  input: frequency
 *  Returns: index
 */
int32_t rtc_set_freq(int freq){
    char hex_rate;

    /*Check to see if the input frequency is divisible by 2*/
    if( (freq % 2) != 0){
      printf("Error, input frequency not divisible by 2\n");
      return -1;
    }

    /*Turn the frequency into a hex rate that can be sent to RTC*/
    if(freq == 2){
        hex_rate = 0x0F;
    }
    else if(freq == 4){
        hex_rate = 0x0E;
    }
    else if(freq == 8){
        hex_rate = 0x0D;
    }
    else if(freq == 16){
        hex_rate = 0x0C;
    }
    else if(freq == 32){
        hex_rate = 0x0B;
    }
    else if(freq == 64){
        hex_rate = 0x0A;
    }
    else if(freq == 128){
        hex_rate = 0x09;
    }
    else if(freq == 256){
        hex_rate = 0x08;
    }
    else if(freq == 512){
        hex_rate = 0x07;
    }
    else if(freq == 1024){
        hex_rate = 0x06;
    }
    else {
        return -1;
    }
    
    /*Mask Interrupts*/
    cli();

    /*Disable NMI*/
    outb(RTC_REGA, RTC_PORT);		

    /*Recieve the Initial Value from Reg. A*/
    char prev = inb(CMOS_PORT);	

    /*Resets index to Register A*/
    outb(RTC_REGA, RTC_PORT);		

    /*Writes the rate to Register A*/
    outb(((prev & TOP_4_BITS) | hex_rate), CMOS_PORT);
    
    /*Re-Enable Interrupts*/
	  sti();                    
	  return 1;
}

/*
*    rtc_write
*  Inputs: file_index not used (for file operations table), buffer of frequency, the number of input bytes
*  Returns: 0 --success
*          -1 --failure
*/
int32_t rtc_write(int32_t file_index, const void *buf, int32_t nbytes){ 
    int set_freq_return;
    /*Check if buffer is bull*/
    if (buf == NULL) {   
        printf("Null buffer passed into rtc_write!\n");                   
        return -1;
    }
    
    set_freq_return = rtc_set_freq(*((int32_t*) buf));  // change frequency but first convert input to int32_t
    if(set_freq_return != 0){
        return -1;
    }

    return 0;                           // else return success
}

/* rtc_read: return when rtc interrupt
 * Inputs: none
 * Outputs: none
 */
int32_t rtc_read(int32_t file_index, void* buf, int32_t nbytes){
    
    /*Variable that stores flags*/
    int saved_flags;

    /*Saves current flags and disables interrupts*/
    cli_and_save(saved_flags);

    in_count = 0; // resets the interrupt flag to low
    
    /*Re-enables interrupts*/
    sti();

    /*Waits until the interrupt flag goes high*/
    while(in_count == 0){

    }            
    /*Restores flags*/  
    restore_flags(saved_flags);


    return 0;
}


/*
* rtc_open
* initializes RTC frequency to 2HZ, return 0
*  Inputs: const uint8_t* fname -- not used (for file operations table)
*  Returns: 0 --success
*          -1 --failure
*/
int32_t rtc_open(const uint8_t* fname){ 
    /*We want the initial frequency to be 2Hz*/
    int initial_freq = 2;
    int open_ret;
    
    /*Sets frequency and evaluates return value*/
    open_ret = rtc_set_freq(initial_freq);

    if (open_ret != 0){
      printf("RTC Open failure!\n");
      return -1;
    }
    return 0;
}

/*rtc_close()
* Inputs: int32_t file_index -- not used (for file operations table)
* Outputs: none
*/
int32_t rtc_close(int32_t file_index){
    return 0;
}
