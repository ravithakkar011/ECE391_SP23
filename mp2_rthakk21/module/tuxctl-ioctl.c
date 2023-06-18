/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)


//Initializing ioctl functions
int tux_init(struct tty_struct* tty);
int tux_buttons(struct tty_struct* tty, unsigned long arg);
int tux_set_led(struct tty_struct* tty, unsigned long arg);

//Initializing helper function for tux_set_led
unsigned char hex_num_to_byte(unsigned char input);


//Delcaring and Initializing Global Variables:
//---------------------------------------------

// Acknowledge Flag. High means acknowledge recieved
int ack_flag = 1; 

//Global variable that holds the button byte with corresponding values  [right, left, down, up C, B , A, START]
unsigned char button_val[1];

//Global variable that holds the previous state of the LEDs
unsigned long prev_state_leds;

//Bit masks
unsigned char BITWISE_MASK = 0x0F; //  Value corresponding to 00001111 will be used throughout the file
unsigned char BITWISE_MASK2 = 0x08; // Value corresponding to 00001000 will be used throughout the file to extract bit values
unsigned long BITWISE_MASK3 = 0x0000FFFF; // Value corresponding to 00000000000000001111111111111111 and is used to extract the first 16 bits of the set led argument

//Seven Segment Display Byte Values (dp bit low)
unsigned char SS0 = 0xE7;
unsigned char SS1 = 0x06;
unsigned char SS2 = 0xCB;
unsigned char SS3 = 0x8F;
unsigned char SS4 = 0x2E;
unsigned char SS5 = 0xAD;
unsigned char SS6 = 0xED;
unsigned char SS7 = 0x86;
unsigned char SS8 = 0xEF;
unsigned char SS9 = 0xAF;
unsigned char SSA = 0xEE;
unsigned char SSB = 0x6D;
unsigned char SSC = 0xE1;
unsigned char SSD = 0x4F;
unsigned char SSE = 0xEA;
unsigned char SSF = 0xE8;


/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c; 
	int right, left, down, up;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	switch(a) {
	case MTCP_BIOC_EVENT: 
/*	 MTCP_BIOC_EVENT	
;	Generated when the Button Interrupt-on-change mode is enabled and 
;	a button is either pressed or released.
;
; 	Packet format:
;		Byte 0 - MTCP_BIOC_EVENT
;		byte 1  +-7-----4-+-3-+-2-+-1-+---0---+
;			| 1 X X X | C | B | A | START |
;			+---------+---+---+---+-------+
;		byte 2  +-7-----4-+---3---+--2---+--1---+-0--+
;			| 1 X X X | right | down | left | up |
;			+---------+-------+------+------+----+
*/	
		b = b & BITWISE_MASK;		// bitwise AND operation on b using 00001111
		c = c & BITWISE_MASK; 	// bitwise AND operation on c using 00001111
		right = c & BITWISE_MASK2; 	// bitwise AND operation on c using 00001000
		down = c & (BITWISE_MASK2 >> 1); 	//bitwise AND operation on c using 00000100
		down = down >> 1; // right shifts bit by 1 bit
		left = c & (BITWISE_MASK2 >> 2); 	// bitwise AND operation on c using 00000010
		left = left << 1; // left shifts bit by 1 bit
		up =  c & (BITWISE_MASK2 >> 3); 	// bitwise AND operation on C using 00000001 
		c =  right + left + down + up; // reconstructs c so the order is now: right, left, down, up
		c = c << 4; 	// left shifts b by 4 bits
		button_val[0] = b | c; // bitwise OR operation on b and c is the final bitwise operation that leaves the bits in the correct format. 

	case MTCP_ACK: 
		ack_flag = 1; //Sets acknowledge bit to high meaning acknowledge recieved
	case MTCP_RESET: // 
		tux_init(tty); //reinitialize tux by turning on interrupt mode for buttons and user mode for LEDs
		tux_set_led(tty, prev_state_leds); //set_led must be set with last value written to LEDs
	default:
		break;
	}
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	int ioctl_ret;
    switch (cmd) {
	case TUX_INIT: //Initializes tux by sending two commands
		ioctl_ret = tux_init(tty);
		break;
	case TUX_BUTTONS: //function for polling button presses
		ioctl_ret = tux_buttons(tty, arg);
		break;
	case TUX_SET_LED: //function for setting LED displays
		if (ack_flag == 1){ //if acknowledge has been recieved, set it back to low and save the argument as the previous state
			ack_flag = 0;
			prev_state_leds = arg;
			ioctl_ret = tux_set_led(tty, arg); //calls ioctl function
		}
		else{ //if acknowledge is low just return ioctl ret
			ioctl_ret = 0;
		}
		break;
	case TUX_LED_ACK: 
		return -EINVAL;
	case TUX_LED_REQUEST:
		return -EINVAL;
	case TUX_READ_LED:
		return -EINVAL;
	default:
	    return -EINVAL;
    }
	return ioctl_ret;
}

/* 
 * tux_init 
 *   DESCRIPTION: Initializes any variables associated with the driver. 
 *   Sends MTCP_BIOC_ON and MTCP_LED_USR commands to tux.
 *   INPUTS: tty
 *   OUTPUTS: returns 0
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Initializes variables
 */
int tux_init(struct tty_struct* tty){
	//initializes buffer that is big enough for 2 commands
	unsigned char init_buf[TUX_INIT_NUM_CMDS];
	
	//Enable button interrupt on change
	init_buf[0] = MTCP_BIOC_ON;
	
	//Sending this command allows MTCP_LED_SET to work
	init_buf[1] = MTCP_LED_USR;
	
	tuxctl_ldisc_put(tty, init_buf, TUX_INIT_NUM_CMDS);
	//sets ack_flag high
	ack_flag = 1;
	return 0;
}

/* 
 * tux_buttons
 *   DESCRIPTION: Accesses button_val and copies to user to be accessed by mazegame.c
 *   INPUTS: tty, pointer stored in argument
 *   OUTPUTS: returns 0 if success, -EINVAL if fail
 *   RETURN VALUE: returns 0 if success, -EINVAL if fail
 *   SIDE EFFECTS: copies contents of button_val into user space at the memory address specified by arg.
 */
int tux_buttons(struct tty_struct* tty, unsigned long arg){
	int copy_ret;
	unsigned char tux_byte;
	//checks if pointer is valid
	if (arg == 0){
		return -EINVAL;
	}
	//switches to local variable
	tux_byte = button_val[0];
	//use copy to user to send button value to mazegame.c
	copy_ret = copy_to_user((unsigned long*)arg, &(tux_byte), sizeof(tux_byte));
	//checks if copy was valid
	if (copy_ret != 0){
		return -EINVAL;
	}
	return 0;
}

/* 
 * tux_set_led
 *   DESCRIPTION: ioctl function that sends signals to the tux controller to set what 
 * 	hexadecimal value should be displayed on each of the 4 seven-segment displays.
 *   INPUTS: tty, arg
 *   OUTPUTS: returns 0
 *   RETURN VALUE: returns 0
 *   SIDE EFFECTS: sends LED digits to tux controller
 */

/*
The argument is a 32-bit integer of the following form: The low 16-bits specify a number whose
hexadecimal value is to be displayed on the 7-segment displays. The low 4 bits of the third byte
specifies which LED’s should be turned on. The low 4 bits of the highest byte (bits 27:24) specify
whether the corresponding decimal points should be turned on. This ioctl should return 0.
*/
int tux_set_led(struct tty_struct* tty, unsigned long arg){
	unsigned char led3_status, led2_status, led1_status, led0_status; //holds whether each seven-segment display should be on or off
	unsigned char led3_hex, led2_hex, led1_hex, led0_hex; // holds hex value for each seven-segment display
	unsigned char d3, d2, d1, d0; // Holds decimal bits for each seven-segment display
	unsigned long hex_vals, hex_vals_shifted; 
	unsigned long extract_leds;
	unsigned long extract_decimals;
	unsigned char led_set_buf[LED_SET_NUM_CMDS];



	//setting first byte to the command itself
	led_set_buf[0] = MTCP_LED_SET;

	hex_vals = arg; // performs bitwise AND operation on argument to extract low 16 bits.

	//printk("hex_vals : %lx \n", hex_vals);

	hex_vals_shifted = hex_vals; //hex_vals_shifted acts as a tenporary variable

	// Extracts hex values for every digit and shifts the temporary hex_vals_shifted variable by 4
	led0_hex = hex_vals_shifted & EXTRACT_HEX_AND; //Performs bitwise AND operation to extract hex value for LED 0.
	hex_vals_shifted = hex_vals_shifted >> EXTRACT_HEX_SHIFT;
	
	led1_hex = hex_vals_shifted & EXTRACT_HEX_AND;
	hex_vals_shifted = hex_vals_shifted >> EXTRACT_HEX_SHIFT;

	led2_hex = hex_vals_shifted & EXTRACT_HEX_AND;
	hex_vals_shifted = hex_vals_shifted >> EXTRACT_HEX_SHIFT;

	led3_hex = hex_vals_shifted & EXTRACT_HEX_AND;
	
	//extract which LEDs are on and which are off
	extract_leds = BITWISE_MASK << LEDS_MSB; //left shift the bitwise mask by 16 bits to extract which LEDs should be turned on
	extract_leds = extract_leds & arg; //extract
	extract_leds = extract_leds >> LEDS_MSB; //Right shift back to the least significant byte
	led_set_buf[1] = extract_leds;

	//find on/off status for each led digit
	led3_status = extract_leds & BITWISE_MASK2; //Performs AND operation with extract_leds and 00001000 to extract 3rd bit
	led3_status = led3_status >> 3; //Makes LSB the status bit

	led2_status = extract_leds & (BITWISE_MASK2 >> 1); //Performs AND operation with extract_leds and 00000100 to extract 2nd bit
	led2_status = led2_status >> 2; //Makes LSB the status bit

	led1_status = extract_leds & (BITWISE_MASK2 >> 2); //Performs AND operation with extract_leds and 00000010 to extract 1st bit
	led1_status = led1_status >> 1; //Makes LSB the status bit

	led0_status = extract_leds & (BITWISE_MASK2 >> 3); //Performs AND operation with extract_leds and 00000001 to extract 0th bit

	//Call hex_num_to_byte
	led_set_buf[2] = hex_num_to_byte(led0_hex); 
	led_set_buf[3] = hex_num_to_byte(led1_hex);
	led_set_buf[4] = hex_num_to_byte(led2_hex);
	led_set_buf[5] = hex_num_to_byte(led3_hex);

	//find which decimal points need to be turned on
	extract_decimals = BITWISE_MASK << DECIMALS_MSB; //left shift the bitwise mask by 24 bits to extract which decimal points need to be turned on 
	extract_decimals = extract_decimals & arg;
	extract_decimals = extract_decimals >> DECIMALS_MSB; // Right shift back to the least significant byte

	//find decimal status for each seven segment LED display
	d3 = extract_decimals & BITWISE_MASK2; //Performs AND operation with extract_decimals and 00001000 to extract 3rd bit
	d3 = d3 << 1; // Left shifts the decimal bit to the correct position (4th bit)
	d2 = extract_decimals & (BITWISE_MASK2 >> 1); //Performs AND operation with extract_decimals and 00000100 to extract 2nd bit
	d2 = d2 << 2;
	d1 = extract_decimals & (BITWISE_MASK2 >> 2); //Performs AND operation with extract_decimals and 00000010 to extract 1st bit
	d1 = d1 << 3;
	d0 = extract_decimals & (BITWISE_MASK2 >> 3); //Performs AND operation with extract_decimals and 00000001 to extract 0th bit
	d0 = d0 << 4;
	
	//add decimal value to total value
	led_set_buf[2] += d0;
	led_set_buf[3] += d1;
	led_set_buf[4] += d2;
	led_set_buf[5] += d3;

	//Overwrite return values for digit if led status == 0; 
	if (led0_status == 0){
		led_set_buf[2] = 0x00;
	}
	if (led1_status == 0){
		led_set_buf[3] = 0x00;
	}
	if (led2_status == 0){
		led_set_buf[4] = 0x00;
	}
	if (led3_status == 0){
		led_set_buf[5] = 0x00;
	}
	//Sending buffer to tux 

	//TEST
	// led_set_buf[0] = MTCP_LED_SET;
	// led_set_buf[1] = 0x0F;
	// led_set_buf[2] = 0x2E;
	// led_set_buf[3] = 0x2E;
	// led_set_buf[4] = 0xEF;
	// led_set_buf[5] = 0xEF;

	tuxctl_ldisc_put(tty, led_set_buf, LED_SET_NUM_CMDS); // send bytes to tux to set LEDs
	//ack_flag = 1;
	return 0;
}


/* 
 * hex_num_to_byte
 *   DESCRIPTION: A helper function that maps each hexadecimal value 0-F with a byte that formats the digit for the tux controller,
 * according to the following graphic:
 * ; 	Mapping from 7-segment to bits
; 	The 7-segment display is:
;		  _A
;		F| |B
;		  -G
;		E| |C
;		  -D .dp
;
; 	The map from bits to segments is:
; 
; 	__7___6___5___4____3___2___1___0__
; 	| A | E | F | dp | G | C | B | D | 
; 	+---+---+---+----+---+---+---+---+
 *   INPUTS: 1-digit hexadecimal input
 *   OUTPUTS: returns a mapped 7-segment byte
 *   RETURN VALUE: a mapped 7-segment byte
 *   SIDE EFFECTS: None
 */
unsigned char hex_num_to_byte(unsigned char input){
	/*
	In this helper function, I created an array with pre-defined bytes to return based on the hexadecimal input. 
	The way this works is that the array is initialized, and the input hex digit acts as the index to the array. 
	I calculated the values in the array manually, and they correspond to the byte for the 7 segment display. 
	The byte does not keep the dp bit high. If it is indicated in the argument that the decimal point should be shown for a given 
	LED 7-segment display, it will be added after a call to this function. 
	*/
	unsigned char display_byte_array[16] = {SS0, SS1, SS2, SS3, SS4, SS5, SS6, SS7, SS8, SS9, SSA, SSB, SSC, SSD, SSE, SSF};
	return display_byte_array[input];
}
