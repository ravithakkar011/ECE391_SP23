/* terminal_driver.h - Defines & headers used to for terminal_driver.c
 */

#ifndef _TERMINAL_DRIVER_H
#define _TERMINAL_DRIVER_H

#include "keyboard.h"
#include "types.h"
#include "lib.h"
#include "syscall.h"
#include "paging.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7
#define NUM_TERMINALS   3




typedef struct terminal_struct{
    
    uint32_t video_mem_addr;                        // stores starting address of the terminal's video memory page
    uint32_t active; //1 if active, 0 if not
    uint8_t keyboard_buffer[128];
    uint32_t screen_X;
    uint32_t screen_Y;
   
}terminal_t; //process control block

int curr_terminal_id;           // initi in terminal init
terminal_t terminals[NUM_TERMINALS];

//delcaring driver functions
int32_t terminal_init(uint32_t terminal_id); 
int32_t terminal_open(const uint8_t* fname); 
int32_t terminal_close(int32_t file_index); 
int32_t terminal_read(int32_t file_index, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t file_index, const void* buf, int nbytes);
int32_t terminal_switch(uint32_t terminal_id);


#endif
