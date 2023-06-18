#include "paging.h"

extern void enable(uint32_t directory);

// /* Initialize Paging */
void paging_init(void)
{
    int i;//loop number

    //set up first 4MB with 4KB table
    page_directory_entry_t pde;
    pde.present            = 1;
    pde.read_write         = 1;
    pde.user               = 0;
    pde.write_through      = 0;
    pde.cache_disable      = 0;
    pde.accessed           = 0;
    pde.reserved           = 0;
    pde.size               = 0;//4KB
    pde.ignored            = 0;
    pde.avail              = 0;
    pde.table_addr         =((uint32_t)kernel_page_table) >> 12;//the address needs to move to highest 20 bits

    kernel_page_directory[0] = pde;

    //set up 4MB-8MB
    kernel_page_directory[1].present            = 1;
    kernel_page_directory[1].read_write         = 1;
    kernel_page_directory[1].user               = 0;
    kernel_page_directory[1].write_through      = 0;
    kernel_page_directory[1].cache_disable      = 0;
    kernel_page_directory[1].accessed           = 0;
    kernel_page_directory[1].reserved           = 0;
    kernel_page_directory[1].size               = 1;//4MB
    kernel_page_directory[1].ignored            = 0;
    kernel_page_directory[1].avail              = 0;
    kernel_page_directory[1].table_addr         = SIZE_4MB >> 12;//kernel addr,4MB of virtual memory----4MB of physical memory

    //set up the rest
    for (i=2 ; i < SPACE ; i++){
        kernel_page_directory[i].present            = 0;
        kernel_page_directory[i].read_write         = 1;
        kernel_page_directory[i].user               = 0;
        kernel_page_directory[i].write_through      = 0;
        kernel_page_directory[i].cache_disable      = 0;
        kernel_page_directory[i].accessed           = 0;
        kernel_page_directory[i].reserved           = 0;
        kernel_page_directory[i].size               = 1;//4MB
        kernel_page_directory[i].ignored            = 0;
        kernel_page_directory[i].avail              = 0;
        kernel_page_directory[i].table_addr         = i;
    }

    //pte
    for (i=0 ; i < SPACE ; i++){
        if (i==VIDEO_ADDR){
            kernel_page_table[i].present            = 1;
            kernel_page_table[i].read_write         = 1;
            kernel_page_table[i].user               = 0;
            kernel_page_table[i].write_through      = 0;
            kernel_page_table[i].cache_disable      = 0;
            kernel_page_table[i].accessed           = 0;
            kernel_page_table[i].dirty              = 0;
            kernel_page_table[i].reserved           = 0;
            kernel_page_table[i].global             = 0;
            kernel_page_table[i].avail              = 0;
            kernel_page_table[i].page_addr          = VIDEO_ADDR;
        }
        else{
            kernel_page_table[i].present            = 0;
            kernel_page_table[i].read_write         = 1;
            kernel_page_table[i].user               = 0;
            kernel_page_table[i].write_through      = 0;
            kernel_page_table[i].cache_disable      = 0;
            kernel_page_table[i].accessed           = 0;
            kernel_page_table[i].dirty              = 0;
            kernel_page_table[i].reserved           = 0;
            kernel_page_table[i].global             = 0;
            kernel_page_table[i].avail              = 0;
            kernel_page_table[i].page_addr          = i;
        }    
    }
    enable((uint32_t)kernel_page_directory);
}
/*Function: execute_paging_init ( uint32_t pid )
 *Description: set up the page directory for the program
 *Input: pid
 *Output: none
 *Side effect: also flush the TLB using flush_tlb()
* calculate the physical address of the page
 */
void execute_paging_init(uint32_t pid){
    int phy_addr = SIZE_8MB + (SIZE_4MB * (pid - 1));
    kernel_page_directory[PROGRAM_IMG_PDE_INDEX].present = 1;
    kernel_page_directory[PROGRAM_IMG_PDE_INDEX].user= 1;
    kernel_page_directory[PROGRAM_IMG_PDE_INDEX].table_addr = (phy_addr/SIZE_4KB/SPACE) << 10;

    flush_tlb((int)kernel_page_directory);
}

/*Function: map_vidmem ( )
 *Description: map the video memory to the user space
 *Input: none
 *Output: none
 *Side effect: also flush the TLB using flush_tlb()
 */
void map_vidmem(){
    kernel_page_directory[VMEM_PDE_INDEX].present = 1;
    kernel_page_directory[VMEM_PDE_INDEX].user= 1;
    kernel_page_directory[VMEM_PDE_INDEX].size = 0; //4KB
    kernel_page_directory[VMEM_PDE_INDEX].read_write = 1;
    kernel_page_directory[VMEM_PDE_INDEX].table_addr = (unsigned int)pagetable_video >> 12;

    pagetable_video[0].present = 1;
    pagetable_video[0].user= 1;
    pagetable_video[0].read_write = 1;
    pagetable_video[0].page_addr = VIDEO_ADDR;
    
    flush_tlb((int)kernel_page_directory);
}

void terminal_videopage_init(){

    /* Init video memory page for first terminal*/
    int32_t t1 = VIDEO_ADDR + 1; 
    kernel_page_table[(t1)].present          = 1;
    kernel_page_table[(t1)].read_write       = 1;
    kernel_page_table[(t1)].user             = 0;
    kernel_page_table[(t1)].write_through    = 0;
    kernel_page_table[t1].cache_disable      = 0;
    kernel_page_table[t1].accessed           = 0;
    kernel_page_table[t1].dirty              = 0;
    kernel_page_table[t1].reserved           = 0;
    kernel_page_table[t1].global             = 0;
    kernel_page_table[t1].avail              = 0;
    kernel_page_table[t1].page_addr          = (t1);

    /* Init video memory page for second terminal*/
    int32_t t2 = VIDEO_ADDR + 2; 
    kernel_page_table[t2].present            = 1;
    kernel_page_table[t2].read_write         = 1;
    kernel_page_table[t2].user               = 0;
    kernel_page_table[t2].write_through      = 0;
    kernel_page_table[t2].cache_disable      = 0;
    kernel_page_table[t2].accessed           = 0;
    kernel_page_table[t2].dirty              = 0;
    kernel_page_table[t2].reserved           = 0;
    kernel_page_table[t2].global             = 0;
    kernel_page_table[t2].avail              = 0;
    kernel_page_table[t2].page_addr          = (t2);

    int32_t t3 = VIDEO_ADDR + 3;
    /* Init video memory page for third terminal*/
    kernel_page_table[t3].present            = 1;
    kernel_page_table[t3].read_write         = 1;
    kernel_page_table[t3].user               = 0;
    kernel_page_table[t3].write_through      = 0;
    kernel_page_table[t3].cache_disable      = 0;
    kernel_page_table[t3].accessed           = 0;
    kernel_page_table[t3].dirty              = 0;
    kernel_page_table[t3].reserved           = 0;
    kernel_page_table[t3].global             = 0;
    kernel_page_table[t3].avail              = 0;
    kernel_page_table[t3].page_addr          = t3;

}
