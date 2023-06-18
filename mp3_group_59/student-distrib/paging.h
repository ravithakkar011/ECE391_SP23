#ifndef _PAGING_H
#define _PAGING_H


#ifndef ASM

#include "types.h"
#include "lib.h"

#define SIZE_4KB 4096
#define SPACE 1024
#define VIDEO 0xB8000
#define VIDEO_ADDR (0xB8000>>12)
#define SIZE_4MB 0x400000
#define SIZE_8MB   0x800000
#define SIZE_128MB 0x8000000 
#define PROGRAM_IMG_PDE_INDEX   32
#define VMEM_PDE_INDEX          40 // 40*4MB = 160MB
#define USER_VMEM_ADDR          0x0a000000 // 160MB

typedef struct __attribute__((packed)) page_directory_entry_t{
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user               : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disable      : 1;
    uint32_t accessed           : 1;
    uint32_t reserved           : 1;
    uint32_t size               : 1;      //4MB or 4kB
    uint32_t ignored            : 1;
    uint8_t  avail              : 3;
    uint32_t table_addr         : 20;    //BASE
}page_directory_entry_t;

//32 bit Entries used for the tables
typedef struct __attribute__((packed)) page_table_entry_t{
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user               : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disable      : 1;
    uint32_t accessed           : 1;
    uint32_t dirty              : 1;
    uint32_t reserved           : 1;
    uint32_t global             : 1;
    uint8_t  avail              : 3;
    uint32_t page_addr          : 20;   //BASE
}page_table_entry_t;

/* Page directory and page tables */
page_directory_entry_t kernel_page_directory[1024] __attribute__((aligned(4096)));
page_table_entry_t kernel_page_table[1024] __attribute__((aligned(4096)));
page_table_entry_t pagetable_video[1024] __attribute__((aligned(4096)));

//initialize paging in kernel
extern void paging_init(void);

//initalize paging for program image
void execute_paging_init(uint32_t pid);

void flush_tlb(int d);

// Initialize paging for video memory
void map_vidmem();

//initialize paging for terminal video memory
void terminal_videopage_init();


#endif /* ASM */
#endif /* _PAGING_H */
