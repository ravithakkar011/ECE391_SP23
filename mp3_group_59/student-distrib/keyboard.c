/* keyboard.c - Manages inreactions between keyboard and display through PICs to output characters to the screen
 * NOTES:
 *  - create functionality for all code up to 0x3A (58 chars)
 */

#include "keyboard.h"

//int kb_buff_len = 0;                  // length of keyboard buffer (different to num_chars_typed b/c of tab)
int ctrl_pressed = 0;                   // bool flag to see if ctrl key was pressed so code can look out for next character
int shift_pressed = 0;                  // bool flag to see if shift key was pressed so code can look out for next character
int capslock_pressed = 0;               // bool flag to see if capslock key was pressed so code can look out for next characters
int alt_pressed = 0;                    // bool flag to see if alt key was pressed so code can look out for next character
//int switch_terminals = 0;               // bool flag to see if alt key was pressed so code can look out for next character
//bool key_pressed = false;             // uncomment for paging test

/* global buffer that holds character history from keyboard */
uint8_t keyboard_buffer[NUM_CHARS_KB];
/* look up table to convert output from keyboard to readable ascii's */
uint8_t ascii_lookup_table[SIZE_KB_CODES]= {                                            // scan codes from keyboard map to certain ascii's according to osdev; keycodes are index for tables; 
  0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 
  DASH, EQUAL, BACKSPACE, TAB, 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 
  0x69, 0x6F, 0x70, L_BRACK, R_BRACK, ENTER, CTRL, 0x61, 0x73, 0x64, 0x66, 
  0x67, 0x68, 0x6A, 0x6B, 0x6C, SEMI_COL, SING_QUOTE, BACK_TICK, SHIFT, 
  BACKSLASH, 0x7A, 0x78, 0x63, 0x76, 0x62, 0x6E, 0x6D, COMMA, PERIOD, 
  FWD_SLASH, SHIFT, 0x00, ALT, SPACE_KEY, CAPSLOCK
};

/* look up table to convert output from keyboard to readable caps ascii's */
uint8_t ascii_caps_lookup_table[SIZE_KB_CODES]= {                                       // scan codes from keyboard map to certain ascii's according to osdev; keycodes are index for tables; 
  0x00, 0x00, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E, 0x26, 0x2A, 0x28, 0x29, 
  0x5F, 0x2B, BACKSPACE, TAB, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 
  0x49, 0x4F, 0x50, 0x7B, 0x7D, ENTER, CTRL, 0x41, 0x53, 0x44, 0x46, 
  0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3A, 0x22, 0x7E, SHIFT, 
  0x7C, 0x5A, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x4D, 0x3C, 0x3E, 
  0x3F, SHIFT, 0x00, ALT, SPACE_KEY, CAPSLOCK
};



/*
  * clear_keyboard_buff
  * DESCRIPTION: clears keyboard buffer
  * INPUTS: none
  * OUTPUS: none
  * RETURN VALUE: none
  * 
  */
void clear_keyboard_buff(void){
  int i;

  for(i = 0; i < NUM_CHARS_KB; i++){
    keyboard_buffer[i] = 0;
  }
  num_chars_typed = 0;
}

/*
  * print_to_screen
  * DESCRIPTION: prints character to screen
  * INPUTS: none
  * OUTPUS: none
  * RETURN VALUE: none
  * 
  */
void print_to_screen(uint8_t key_code, uint8_t key_char){
  /* bounds check: only allow pressed (non-release) keycodes */
	if(key_code <= NUM_KB_CODES){                                       // check to make sure released key codes don't get passed and then outputted
      /* all upper */
      if(shift_pressed == 1 && capslock_pressed == 0){
        key_char = ascii_caps_lookup_table[key_code];
      }
      /* upper or lower */
      else if(shift_pressed == 1 && capslock_pressed == 1){
        if((key_code >= ONE_KEYCODE && key_code <= 0x0F) || (key_code >= 0x1A && key_code <= 0x1D) || (key_code >= 0x27 && key_code <= 0x2B)|| (key_code >= 0x33 && key_code <= 0x3A)){      // if key is a non-alphabetical input
          key_char = ascii_caps_lookup_table[key_code];               //  output is upper
        }
        else{
          key_char = ascii_lookup_table[key_code];                    // otherwise lower
        }
      }
      /* upper or lower */
      else if(shift_pressed == 0 && capslock_pressed == 1){
        if((key_code >= ONE_KEYCODE && key_code <= 0x0F) || (key_code >= 0x1A && key_code <= 0x1D) || (key_code >= 0x27 && key_code <= 0x2B)|| (key_code >= 0x33 && key_code <= 0x3A)){      // if key is a non-alphabetical input
          key_char = ascii_lookup_table[key_code];                    // output is lower
        }
        else{
          key_char = ascii_caps_lookup_table[key_code];               // otherwise upper
        }
      }
      /* all lower */
      else{
        //kb_buff_len++;
        key_char = ascii_lookup_table[key_code];
      }
      
      keyboard_buffer[num_chars_typed] = key_char;
      num_chars_typed++;
      putc(key_char);                                               // outputs char to screen

    /* debug tests */
    //printf("%d", num_chars_typed);
    //printf(" |%c| ", key_char);
    //printf("|%x - %c|", key_code, key_char);
    }
}

/*
  * keyboard_initializer
  * DESCRIPTION: initializes keyboard functionaity by enabling interrupt
  * INPUTS: none
  * OUTPUS: none
  * RETURN VALUE: none
  * 
  */
void keyboard_initialize(void){
	enable_irq(KEYBOARD_IRQ_NUM);										                  // 1 bc that's the IRQ number on IDT
  num_chars_typed = 0;
  clear_keyboard_buff();
  enter_pressed_flag = 0;
}

 /*
  * keyboard_handler
  * DESCRIPTION: displayes just one or a string of characters to the screen
  * INPUTS: none
  * OUTPUS: none
  * RETURN VALUE: none
  * NOTES:
  *		- interrupt	
  *		- get keystroke
  *		- print to screen
  */
void keyboard_handler(void) {
	uint8_t key_char;                   // converted output to screen
  uint8_t key_code;                   // output from the keyboard
  //int num_spaces;                     // loop counter for tab
  uint32_t screen_x, screen_y;
  int i;
  //key_pressed = true;

  cli();                                  // clear interrupt

  key_code = inb(KEYBOARD_DATA_PORT);			// reads from kb data port (char is one byte but inb returns 32)

  switch (key_code){
    case BACKSPACE_KEYCODE :
      if(num_chars_typed > 0){
        num_chars_typed = num_chars_typed - 1;

        screen_x = get_screen_x() - 1;
        screen_y = get_screen_y();
        set_cursor(screen_x, screen_y);                            // move back cursor

        putc(SPACE_KEY);                                           // put a space where last character was             

        screen_x = get_screen_x() - 1;
        screen_y = get_screen_y();
        set_cursor(screen_x, screen_y);                            // move back cursor again to where last character was
      }
      break;
    case TAB_KEYCODE :
      // for(num_spaces = 0; num_spaces <= 4; num_spaces++){     // four spaces for tab
      //   putc(SPACE_KEY);
      // }
      num_chars_typed++;             // = num_chars_typed + 4; for tab case (not necessary)
      keyboard_buffer[num_chars_typed] = '\t';
      break;
    case CTRL_KEYCODE :
      ctrl_pressed = 1;
      break;
     case CTRL_REL_KEYCODE :
      ctrl_pressed = 0;
      break;
    case LETTER_L_KEYCODE :                                   // CTRL-L
      if(ctrl_pressed == 1){
        clear();                                              // clear screen
        set_cursor(0, 0);                                     // put cursor at top of screen
        for(i = 0; i <= num_chars_typed; i++){                 // display what was last in keyboard buff
          putc(keyboard_buffer[i]);
        }

        if(num_chars_typed > MAX_CHARS_LINE){                 // if two lines needed to display buffer
          set_cursor(num_chars_typed - MAX_CHARS_LINE, 1);    // put cursor right after last char in keyboard buff
        }
        else{                                                  // only one line needed
          set_cursor(num_chars_typed, 0);                      // put cursor right after last char in keyboard buff
        }
        // clear buffer?? NO
      }
      else{
        print_to_screen(key_code, key_char);
      }
      break;
    case L_SHIFT_KEYCODE :
      shift_pressed = 1;
      break;
    case R_SHIFT_KEYCODE :
      shift_pressed = 1;
      break;
    case L_SHIFT_REL_KEYCODE :
      shift_pressed = 0;
      break;
    case R_SHIFT_REL_KEYCODE :
      shift_pressed = 0;
      break;
    case ALT_KEYCODE :                                       // for switching terminals (ALT + F2 or F3)
      alt_pressed = 1;
      break;
    case ALT_REL_KEYCODE :
      alt_pressed = 0;
      break;
    case F1_KEYCODE :                                        // for switching terminals
      if(alt_pressed){
        terminal_switch(0);
      }
      break;
    case F2_KEYCODE :                                        // for switching terminals
      if(alt_pressed){
        terminal_switch(1);
      }
      break;
    // case F2_REL_KEYCODE :
    //     alt_pressed = 0;
    //     break;
    case F3_KEYCODE :                                        // for switching terminals
      if(alt_pressed){
        terminal_switch(2);
      }
      break;
    // case F3_REL_KEYCODE :
    //     alt_pressed = 0;
    //     break;
    case CAPSLOCK_KEYCODE :
      capslock_pressed = !capslock_pressed;                   // toggle from last state
      break;
    case ENTER_KEYCODE :
      enter_pressed_flag = 1;
      keyboard_buffer[num_chars_typed] = '\n';
      num_chars_typed++;
      putc('\n');                                             // newline
      break;
   default :
    print_to_screen(key_code, key_char);
    break;
  }

	sti();                              // set interrupt
	send_eoi(KEYBOARD_IRQ_NUM);         // end of interrupt
}
