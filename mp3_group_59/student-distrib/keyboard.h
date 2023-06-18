/* keyboard.h - Defines & headers used to interact with keyboard and display characters
 * NOTES:
 *	- port #'s from osdev wiki
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEYBOARD_DATA_PORT		0x60
#define KEYBOARD_CMD_PORT		0x64
#define MASTER_PIC_PORT			0x20
#define SLAVE_PIC_PORT			0xA0

#define KEYBOARD_IRQ_NUM		1
#define MAX_CHARS_LINE			80                      // max number of characters that can be written to a single line on the terminal
#define NUM_CHARS_KB            128                     // max number of bytes (a single char (one byte))
#define NUM_KB_CODES            0x3A
#define SIZE_KB_CODES           NUM_KB_CODES + 1        // plus one for empty unmapped val at beginning of scan code set

/* special characters' & inputs ascii values */         // special inputs are zero (use names to keep track of where they are in the lookup table)
#define BACKSPACE               0x08
#define TAB                     0x09
#define DASH                    0x2D
#define EQUAL                   0x3D
#define L_BRACK                 0x5B
#define R_BRACK                 0x5D
#define ENTER                   0x00
#define CTRL                    0x00
#define SEMI_COL                0x3B
#define SING_QUOTE              0x27
#define BACK_TICK               0x60
#define SHIFT                   0x0F
#define BACKSLASH               0x5C
#define COMMA                   0x2C
#define PERIOD                  0x2E
#define FWD_SLASH               0x2F
#define ASTRIX                  0x2A
#define ALT                     0x00
#define SPACE_KEY               0x20
#define CAPSLOCK                0x00

/* special inputs' scan codes */
#define BACKSPACE_KEYCODE               0x0E
#define TAB_KEYCODE                     0x0F
#define CTRL_KEYCODE                    0x1D        // left ctrl
#define CTRL_REL_KEYCODE                0x9D        // left ctrl released
#define L_SHIFT_KEYCODE                 0x2A        // left shift
#define R_SHIFT_KEYCODE                 0x36        // right shift
#define L_SHIFT_REL_KEYCODE             0xAA        // left shift released
#define R_SHIFT_REL_KEYCODE             0xB6        // right shift released
#define ALT_KEYCODE                     0x38        // left alt
#define ALT_REL_KEYCODE                 0xB8
#define F1_KEYCODE                      0x3B
#define F2_KEYCODE                      0x3C
#define F3_KEYCODE                      0x3D
#define CAPSLOCK_KEYCODE                0x3A
#define ENTER_KEYCODE                   0x1C
#define LETTER_L_KEYCODE                0x26        // for checking for ctrl-L to clear screen
#define SPACE_KEYCODE                   0x39
#define ONE_KEYCODE                     0x02        // for checking if key is in a non-numerical input range (caps + shift lock case)


#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal_driver.h"

/* Global Variables */
extern uint8_t keyboard_buffer[NUM_CHARS_KB];                  // global buffer that holds character history from keyboard
extern uint8_t ascii_lookup_table[SIZE_KB_CODES];              // convert the output from the keyboard to ascii's readable by the screen
extern uint8_t ascii_caps_lookup_table[SIZE_KB_CODES];         // convert the output from the keyboard to caps version of ascii's readable by the screen
volatile int enter_pressed_flag;
volatile int num_chars_typed;                    // number of characters typed (account for backspace)

// extern int num_chars_typed;                                 // number of characters typed (account for backspace)
// extern bool shift_pressed;                                  // flag to see if shift key was pressed so code can look out for next character
// extern bool ctrl_pressed;                                   // flag to see if ctrl key was pressed so code can look out for next character
// extern bool capslock_pressed;                               // flag to see if capslock key was pressed so code can look out for next characters
//extern bool key_pressed;

/* Outputs character from keyboard processed by handler to screen */
void print_to_screen(uint8_t key_code, uint8_t key_char);

/* Clear keyboard buffer */
void clear_keyboard_buff(void);

/* Initializer for keyboard interrupt */
void keyboard_initialize(void);

/* Handler for keyboard inputs & outputting to screen */
void keyboard_handler(void);

#endif
