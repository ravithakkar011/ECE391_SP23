// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)

//Defining Constants used in tuxctl-ioctl.c
#define TUX_INIT_NUM_CMDS 2 //We send 2 commands when we initialize the tux
#define LED_SET_NUM_CMDS 6 // We send 6 bytes when we set the LEDs
#define LEDS_MSB 16 //This is the number of bits we need to shift by to extract the LEDs that need to be on
#define DECIMALS_MSB 24 //This is the number of bits we need to shift by to extract the decimals we need to keep on
#define EXTRACT_HEX_SHIFT 4 // This is the number of bit shifts to right shift a new hex value to be extracted from the argument
#define EXTRACT_HEX_AND 0x000F // This constant is used to AND 16 bits to extract the 4 least significant bits(1 hex value)



#endif


