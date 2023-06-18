/* syscall.h - Defines & headers used to facilitate system calls from systemcall_header.S
 * NOTES: structs are defined in filesystem.h because they are used in both filesystem.c and syscall.c
 *	- 
 */

#ifndef _SYSCALL_H
#define _SYSCALL_H


#include "types.h"
#include "lib.h"
#include "filesystem.h"
#include "rtc.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal_driver.h"

#define MAGIC_EXECUTABLE 0x464c457f //ELF
#define KERNEL_END 0x800000     //8MB
#define PAGESIZE_4MB 0x400000 //4MB
#define FISRT_INST_ADDR 24 //24th byte
#define FIRST_INST_ADDR_LEN 4  //4 bytes
#define PROGRAM_IMG_VIRT_ADDR 0x08048000 
#define SIZE_PROGRAM_IMG 0x400000 - 0x48000 //4MB - 0.5MB
#define KERNEL_STACK_WIDTH 0x2000   //8KB
#define IMG_BIG_START 0x8000000 //128MB
#define ASSIGN_PID_ERROR -99 //error code for assign_PID() function
#define FILE_DESC_START_IDX         2   
#define MAX_FILE_DESC_IDX           8 
#define STDIN_INDEX                 0
#define STDOUT_INDEX                1

/*systemcall linkage */
extern void syscall_handler(); //systemcall_header.S

void file_operations_initialize(void); //initialize file operations table

/* Global Pointers to Start of Process Objects */
file_operations_table_t null;          // for when there is no file operator connection
file_operations_table_t stdin;          // terminal
file_operations_table_t stdout;        // terminal
file_operations_table_t rtc;          // rtc
file_operations_table_t files;         // files
file_operations_table_t directories;  // directories

/*System Call Declarations*/
int32_t system_execute(const uint8_t* command); 
int32_t system_halt(const uint8_t status);
int32_t system_read(int32_t fd, void* buf, int32_t nbytes);
int32_t system_write(int32_t fd, void* buf, int32_t nbytes);
int32_t system_open(const uint8_t* filename);
int32_t system_close(int32_t fd);
int32_t system_getargs(uint8_t* buf, int32_t nbytes);
int32_t system_vidmap(uint8_t** screen_start);

/*helper function declarations*/
int32_t find_PCB(int32_t pid);
int32_t assign_PID();




#endif
