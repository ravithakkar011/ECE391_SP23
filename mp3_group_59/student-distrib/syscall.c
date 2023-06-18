/* syscall.c - Contains each system call function that is called by systemcall_handler.S
 *  Functions:      system_execute(const uint8_t* command)
    *              system_halt(const uint8_t status)
    *             system_read(int32_t fd, void* buf, int32_t nbytes)
    *            system_write(int32_t fd, void* buf, int32_t nbytes)
    *           system_open(const uint8_t* filename)
    *         system_close(int32_t fd)
    *       system_getargs(uint8_t* buf, int32_t nbytes)
    *    system_vidmap(uint8_t** screen_start)
    * file_operations_initialize(void)
    * find_PCB(int32_t pid)
    * assign_PID()
    * 
 */

#include "syscall.h"

/*Magic Bytes for bytes 0-3 in file*/
#define MAGIC_BYTE_0    0x7f 
#define MAGIC_BYTE_1    0x45 
#define MAGIC_BYTE_2    0x4c
#define MAGIC_BYTE_3    0x46
#define BYTES_TO_CMPR   4  

/*Numerical Constants*/
#define MAX_PROCESSES       5 //max number of processes (as told by TA)
#define END_OF_KERNEL_PAGE  0x800000 //8MB
#define KERNEL_STACK_SIZE   0x2000 //8kB



/*global variables*/
int32_t pid; //keeps track of current process ID
int32_t parent_pid; //keeps track of parent process ID


/*Function Name: file_operations_initialize(void)
    * Description: Initializes the file operations table
    * Inputs: none
    * Outputs: none
    * Side Effects: Initializes the file operations table
*/
void file_operations_initialize(void){
    // file_operations_table_t null;          // for when there is no file operator connection
    null.read = no_operation_read; 
    null.write = no_operation_write;    
    null.open = no_operation_open;
    null.close = no_operation_close;

    // file_operations_table_t stdin;          // terminal
    stdin.read = terminal_read;
    stdin.write = no_operation_write;              // just reads from terminal
    stdin.open = terminal_open;
    stdin.close = terminal_close;

    // file_operations_table_t stdout;
    stdout.read = no_operation_read;
    stdout.write = terminal_write;  
    stdout.open = terminal_open;
    stdout.close = terminal_close;

    // file_operations_table_t rtc; 
    rtc.read = rtc_read;    
    rtc.write = rtc_write;
    rtc.open = rtc_open;
    rtc.close = rtc_close;

    // file_operations_table_t files;
    files.read = file_read;
    files.write = file_write;
    files.open = file_open;
    files.close = file_close;

    // file_operations_table_t directories;
    directories.read = directory_read;
    directories.write = directory_write;
    directories.open = directory_open;
    directories.close = directory_close;

}



/*
 *	Function: system_execute(const uint8_t* command)
 *	Description: Executes a user level program by performing various tasks before context switch.

 1. Parses command from string
 2. Separates command from arguments
 3. Searches filesystem for file name corresponding to program
 4. Reads starting bits of program image in order to determine if it's an executable
 5. Assigns pid to process and keeps track of the number of active processes
 6. Initializes a PCB struct for the process
 7. Sets up paging for the program image
 8. Copies program image to virtual memory address
 9. Saves relevant info to PCB, edits TSS
 10. Pushes required values into stack, executes IRET to perform context switch

 *	inputs:		command -- execute command, also contains arguments
 *	outputs:	-1 if fail
 *              256 if exceptions
 *              others with other meaning
 *  Side effects: User level context switch performed
 */

int32_t system_execute(const uint8_t* command){
    int ret_val; // value to be returned by function
    int i;
    int32_t parent_pid = pid; // gets global pid value and puts it in this var before its overwritten
    
    /*initial command check*/
    if (command == NULL){
        return -1;
    }

    /*separate command from arguments and spaces*/
    int cmd_strlen = strlen((const int8_t*) command);
    if (cmd_strlen > NUM_CHARS_KB){                             // command length too long
        return -1;
    }
    uint8_t fname[NUM_CHARS_KB];
    uint8_t args[NUM_CHARS_KB];

    /*Initilize the filname buffer and args buffer with \0*/
    for (i = 0;  i < NUM_CHARS_KB ; i++){
        fname[i] = '\0';
        args[i] = '\0';
    }
    
    int arg_starting_index;
    int num_spaces = 0;
    for(i = 0; i < cmd_strlen; i++){
        if(command[i] != ' '){
            fname[i] = command[i];
        }
        else{
            num_spaces++;
            arg_starting_index = i + 1;
        }
    }
    /*Initializes variable to iterate through the characters in the argument*/
    int argument_iterator = arg_starting_index;

    /*Iterates through characters in argument*/
    for (i = 0; i < (cmd_strlen - arg_starting_index); i++){
        if (argument_iterator < cmd_strlen){
            args[i] = command[argument_iterator];
            //printf("Within args loop: %c \n", args[i]);
            argument_iterator++;
        }
        else{
            break;
        }
        
    }
    //printf("system execute- extracted argument: %s \n", args);
    
    /*take file name and see if file exists (read_dentry_by_name)*/

    dentry_t dentry; // pointer to empty dentry to be passed into read_dentry_by_name ::TODO:: may need to be changed to regular struct
    int32_t file_search_ret = read_dentry_by_name(fname, &dentry);
    if (file_search_ret != 0){ //if the file was not found
        // printf("File Search Error!\n");
        return -1;
    }
    
    /*see if the file is executable (compare magic numbers in first 4 bytes of file)*/

    uint32_t file_inode = dentry.inode_num; //extract inode index from dentry
    uint32_t file_length = inodes[file_inode].length;
    uint32_t file_offset = 0; //we want to read from the start of the file
    uint8_t file_bytes[BYTES_TO_CMPR]; //initialize empty buffer for file data
    
    int32_t elf_read_ret = read_data(file_inode, file_offset, file_bytes, BYTES_TO_CMPR); 
    if (elf_read_ret != BYTES_TO_CMPR){ //if the number of bytes copied doesn't match
        // printf("File Read Error! Bytes didn't match \n");
        return -1;
    }

    uint8_t execute_bytes[BYTES_TO_CMPR] = {MAGIC_BYTE_0, MAGIC_BYTE_1, MAGIC_BYTE_2, MAGIC_BYTE_3};

    for (i = 0; i < BYTES_TO_CMPR; i++){ //iterate through the four bytes to see if they're different
        if(file_bytes[i] != execute_bytes[i]){ //if the execute bytes don't match up
            // printf("File Read Error! Execute bytes don't match \n");
            return -1;
        }
    }

    /*keep track of number of active processes using pid*/
    pid = assign_PID();
    if (pid == ASSIGN_PID_ERROR){ //If an error was returned
        printf("Maximum number of processes running!");
        return -1;
    }

    //complete pcb tasks (put at top of kernel stack) 
    
    pcb_obj = (pcb_t*)find_PCB(pid);
    pcb_obj->pcb_pid = pid; //set pid for struct in memory

    //set parent pid within pcb struct
    if(pid == 0){ //if it's already the first process
        pcb_obj->parent_pid = -1;
        parent_pid = -1;
    }
    else{
        pcb_obj->parent_pid = parent_pid;
    }

    pcb_obj->active = 1; //set the pab struct to active

    strncpy((int8_t*)pcb_obj->args, (int8_t*)(args), BYTES_32B);

    /*Set up paging*/
    execute_paging_init(pid+1);    

    //Copy memory from the file to the virtual memory address
    uint32_t read_file_ret = read_data(file_inode, file_offset,(uint8_t*)PROGRAM_IMG_VIRT_ADDR, file_length);

    if (read_file_ret != file_length){ //checks bytes_copied in read_data
        // printf("Error Copying Program Image From File System!");
        return -1;
    }

    //initialize file directory
    for (i = 0; i < MAX_OPEN_FILES; i++){
        pcb_obj->fda[i].fop = &null;
        pcb_obj->fda[i].inode_idx = 0;
        pcb_obj->fda[i].file_position = 0;
        pcb_obj->fda[i].flags = 0;
    }

    /*init stdin and stdout*/
    pcb_obj->fda[STDIN_INDEX].fop = &stdin;
    pcb_obj->fda[STDOUT_INDEX].fop = &stdout;
    pcb_obj->fda[STDIN_INDEX].file_type = 3;
    pcb_obj->fda[STDOUT_INDEX].file_type = 3;


    /*set the fda's to be active*/
    pcb_obj->fda[STDIN_INDEX].flags = 1;
    pcb_obj->fda[STDOUT_INDEX].flags = 1;

    //complete tss (you must alter ESP0 in TSS to contain its new kernel-mode stack pointer, ss0 = Kernel_CS)
    
    tss.ss0 = KERNEL_DS; //points to kernel code segment for 
    tss.esp0 = find_PCB(pid - 1) - 4; //points to process’s kernel-mode stack
    pcb_obj->tss_esp0 = tss.esp0;

    /*save esp and ebp to be used by halt */
    register uint32_t saved_esp asm("esp");
    // printf("EXECUTE: esp of current process: %d\n", saved_esp);
    register uint32_t saved_ebp asm("ebp");
    // printf("EXECUTE: ebp of current process: %d\n", saved_ebp);

   /*Saving these registers are done after paging, done for user context switching*/
    pcb_obj->ebp_val = saved_ebp;
    pcb_obj->esp_val = saved_esp;

    //move exec_pcb to global pcb_obj
    // pcb_obj = exec_pcb;

    //calculate esp for user
    uint32_t user_esp = SIZE_128MB + SIZE_4MB - 4;

    //calculate starting address for eip, stored in bytes 24-27
    uint8_t program_img_starting_addr[FIRST_INST_ADDR_LEN];
    uint32_t starting_address_len_ret = read_data(file_inode, FISRT_INST_ADDR, program_img_starting_addr, FIRST_INST_ADDR_LEN);

    if (starting_address_len_ret != FIRST_INST_ADDR_LEN){
        // printf("Starting Address Length doesn't match!\n");
    }

    uint32_t user_eip = *((int*)program_img_starting_addr);
    //printf("user_eip = %x\n", user_eip);
    sti(); //enable Interrupts
    //printf("Reach\n");
    uint32_t user_ds = USER_DS;
    uint32_t user_cs = USER_CS;
    //execute using iret
    __asm__ volatile(
        "pushl  %0\n"
        "pushl  %1\n"
        "pushfl\n" 
        "pushl  %2\n"
        "pushl  %3\n"
        "iret\n"
        :
        : "g"(user_ds), "g"(user_esp), "g"(user_cs), "g"(user_eip)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    ret_val = 0;
    return ret_val;
}


/*
 *	Function: system_halt(const uint8_t status)
 *	Description: When a user level program returns, the halt function returns control
    back to the previous program that called it, aka a shell. Halt, like execute, 
    performs many tasks.

 1. Evaluates current pid and parent pid , 
 2. If base shell, restarts the shell by calling execute
 3. Finds PCB for parent process
 4. Clears and inactivates current PCB
 5. Fixes TSS
 6. Restores paging for old process and flushes TLB
 7. Replaces current PCB with parent PCB
 8. Returns back to systemcall handler with status and esp and ebp values 

 *	inputs:		status -- status returned from user level program, stored in %ebx but taken
                           in as an argument.
 *	outputs:	an extended 32 bit status value that the systemcall handler takes care of
 *  Side effects: switches back to old process or restarts shell
 */

int32_t system_halt(const uint8_t status){
    // printf("System Halt Reached\n");
    int i;
    uint32_t esp_val;
    uint32_t ebp_val;
    // uint32_t user_ds = USER_DS;
    // uint32_t user_cs = USER_CS;

    //pcb_t* process_ptr = (pcb_t*)(find_PCB(pid));
    int32_t parent_parent_pid;
    // printf("PID in halt (should be 1): %d\n", pid);
    // printf("Parent PID in halt (should be 0): %d\n", parent_pid);
    /*mask interrupts*/
    cli();

    // /*Get ebp and esp values from current pcb */ 
    ebp_val = pcb_obj->ebp_val;
    esp_val = pcb_obj->esp_val;
    // printf("HALT: esp of current process: %d\n", esp_val);
    // printf("HALT: ebp of current process: %d\n", ebp_val);
    /*check if the pid is the base shell (if parent pid is -1)*/

    if(pid == 0 && pcb_obj->parent_pid == -1){ //if so, restart the shell

        /* close current process */
        pcb_obj->active = 0;
        pcb_obj->ebp_val = 0;
        pcb_obj->esp_val = 0;
        pcb_obj->tss_esp0 = 0;
        //pcb_obj->parent_pid = 0;
        for (i = 0;  i < BYTES_32B ; i++){
            pcb_obj->args[i] = '\0';
        }

        /* fix tss */
        tss.ss0 = KERNEL_DS; // do we need to touch this?
        tss.esp0 = KERNEL_END - 4;

        /*redo paging /flushing tlb*/
        execute_paging_init(0);

        /* close file descriptors */
        int i;
        for (i = 0; i < MAX_OPEN_FILES; i++ ) {
        pcb_obj->fda[i].flags = 0;
        }
        
        //pid = -1;
        
        system_execute((uint8_t*)"shell");
    }

    /* find parent process thru PCB */
    pcb_t* parent_proccess_ptr = (pcb_t*)(find_PCB(pcb_obj->parent_pid));    

    /*Clear values for current PCB*/
    pcb_obj->active = 0;
    pcb_obj->ebp_val = 0;
    pcb_obj->esp_val = 0;
    pcb_obj->tss_esp0 = 0;
    for (i = 0;  i < BYTES_32B ; i++){
         pcb_obj->args[i] = '\0';
    }

    /* fix tss */
    tss.ss0 = KERNEL_DS; // do we need to touch this?
    tss.esp0 = KERNEL_END - (KERNEL_STACK_WIDTH * pcb_obj->parent_pid) - 4;

    /* restore paging for parent pid and flushes TLB */
    execute_paging_init(pcb_obj->parent_pid + 1);

    /* Close file descriptors */
    for (i = 0; i < MAX_OPEN_FILES; i++) {
        pcb_obj->fda[i].flags = 0;
        pcb_obj->fda[i].inode_idx = 0;
        pcb_obj->fda[i].file_position = 0;
        pcb_obj->fda[i].fop = &null;
    }
    
    /*new current and parent pids*/
    pid = pcb_obj->parent_pid; //writes parent pid to global variable
    parent_parent_pid = parent_proccess_ptr->parent_pid;
    parent_pid = parent_parent_pid; //writes parent's parent pid to global variable
    pcb_obj = parent_proccess_ptr;
    
    /*re-enable interrupts */
    sti(); 

    /* restore parent data (setup return value)*/
    // printf("Halt Assembly Reached\n");

    uint32_t ret_status = (uint32_t)status;

    if(status == 255){
        ret_status++;
    }

    __asm__ volatile(
        "movl   %0,     %%ebp\n"
        "movl   %1,     %%esp\n"
        "xorl   %%eax,  %%eax\n"
        "movl   %2,     %%eax\n"
        "leave\n"
        "ret\n"
        :
        : "r"(ebp_val), "r"(esp_val), "r"(ret_status)
        : "cc", "%eax", "%esp", "%ebp"
    );

    return -1; // should never reach here
}



/*
*   Function Name: system_open: system_open(const uint8_t* filename)
*   INPUTS: Opens a file based on the file name and file type associated with it
*   OUTPUT: Available file descriptor index of file if successful; -1 if fail
*   NOTES:
*       - file types: 0 -> rtc; 1 -> directory; 2 -> regular file
*/
int32_t system_open(const uint8_t* filename){
    dentry_t dentry_obj;
    int read_dentry_ret = read_dentry_by_name(filename, &dentry_obj);
    if(read_dentry_ret != 0){
        //printf("system_open: Cannot open file: %s \n", filename);
        return -1;
    }

    int fd;

    /* rtc */
    if (dentry_obj.file_type == 0){ // check if rtc
        for (fd = FILE_DESC_START_IDX; fd < MAX_FILE_DESC_IDX; fd++){ 
            if(!(pcb_obj->fda[fd].flags)){                          // check if file descriptor is available
                pcb_obj->fda[fd].inode_idx = dentry_obj.inode_num;
                pcb_obj->fda[fd].file_position = 0;
                pcb_obj->fda[fd].flags = 1;                                         // 1 = file in use 
                pcb_obj->fda[fd].fop = &rtc;
                pcb_obj->fda[fd].file_type = 0;
                //printf("System Open: fd = %d\n", fd);
                return fd;
            }
        }
    } 
    /* directory */
    if (dentry_obj.file_type == 1){ // check if directory
        for (fd = FILE_DESC_START_IDX; fd < MAX_FILE_DESC_IDX; fd++){
            if(!(pcb_obj->fda[fd].flags)){
                pcb_obj->fda[fd].inode_idx = dentry_obj.inode_num;
                pcb_obj->fda[fd].file_position = 0;
                pcb_obj->fda[fd].flags = 1;                                         // 1 = file in use
                pcb_obj->fda[fd].fop = &directories;
                pcb_obj->fda[fd].file_type = 1;
                //printf("System Open: fd = %d\n", fd);
                return fd;
            }
        }
    } 
    /* file */
    if (dentry_obj.file_type == 2){ // check if file
        for (fd = FILE_DESC_START_IDX; fd < MAX_FILE_DESC_IDX; fd++){
            if(!(pcb_obj->fda[fd].flags)){
                pcb_obj->fda[fd].inode_idx = dentry_obj.inode_num;
                pcb_obj->fda[fd].file_position = 0;
                pcb_obj->fda[fd].flags = 1;                                         // 1 = file in use
                pcb_obj->fda[fd].fop = &files;
                pcb_obj->fda[fd].file_type = 2;
                //printf("System Open: fd = %d\n", fd);
                return fd;
            }
        }
    }     
    // printf("system_open: No available file descriptors \n");
    return -1;
}

/*
*   Function Name: system_close
*   INPUTS: Performs the opposite of system open, closes the file directory array input for a given fd
    clears the fop back to null and clears other values
*   OUTPUT: 0 if successful; -1 if fail
*/
int32_t system_close(int32_t fd){
    if (fd < FILE_DESC_START_IDX || fd > (MAX_FILE_DESC_IDX - 1)){
        // printf("system_close: Input file descriptor index out of range \n");
        return -1;}            // check if within range (indices 0 and 1 are reserved for stdin & stdout; cannot close stdin/out)
    if(pcb_obj->fda[fd].flags == 0){return -1;}                                    // check if operations table @ file descriptor is NULL

    pcb_obj->fda[fd].inode_idx = 0;
    pcb_obj->fda[fd].flags = 0;     // close file/file not in use
    pcb_obj->fda[fd].file_position = 0;     // start at beginnig
    pcb_obj->fda[fd].fop = &null;
    pcb_obj->fda[fd].fop->close(fd);
    return 0;
}

/*
*   Function Name: system_read( int32_t fd, void* buf, int32_t nbytes)
*   INPUTS: file descriptor index, buffer, number of bytes
*   OUTPUT: number of bytes read if successful; -1 if fail
*   NOTES:  - reads from the file associated with the file descriptor index
*           - reads nbytes from the file into the buffer
*           - returns the number of bytes read
*           - returns -1 if fail
*/
int32_t system_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("System Read Reached\n");
    if (buf == NULL){
        // printf("system_read: Empty buffer \n");
        return -1;}
    if (fd < STDIN_INDEX || fd > (MAX_FILE_DESC_IDX - 1)){
        // printf("system_read: Input file descriptor index out of range. fd = %d. nbytes = %d \n", fd, nbytes);
        return -1;}             // check if within range (indices 0 and 1 are reserved for stdin & stdout)
    if(pcb_obj->fda[fd].fop == NULL){return -1;}                                    // check if operations table @ file descriptor is NULL
    if(!(pcb_obj->fda[fd].flags)){
        // printf("system_read: File not open \n");
        return -1;}                               // check if file is closed & cannot read (flag = 0)
    return pcb_obj->fda[fd].fop->read(fd, buf, nbytes);                     // select which operation from file_operations_table
}
/*
*   Function Name: system_write (int32_t fd, void* buf, int32_t nbytes)
*   INPUTS: file descriptor index, buffer, number of bytes
*   OUTPUT: number of bytes read if successful; -1 if fail
*   NOTES:  - writes to the file associated with the file descriptor index
*           - writes nbytes from the buffer into the file
*           - returns the number of bytes written
*           - returns -1 if fail
*/
int32_t system_write(int32_t fd, void* buf, int32_t nbytes){
    if (buf == NULL){
        // printf("system_write: Empty buffer \n");
        return -1;}
    if (fd < STDIN_INDEX || fd > (MAX_FILE_DESC_IDX - 1)){
        // printf("system_write: Input file descriptor index out of range. fd = %d \n", fd);
        return -1;}            // check if within range (indices 0 and 1 are reserved for stdin & stdout; cannot close stdin/out)
    if(pcb_obj->fda[fd].fop == NULL){return -1;}                                    // check if operations table @ file descriptor is NULL
    // wasn't getting past this conditional -------> //if(!(pcb_obj->fda[fd].flags)){return -1;}                               // check if file is closed & cannot read (flag = 0)
    //printf("reached \n");
    int ret_val = pcb_obj->fda[fd].fop->write(fd, buf, nbytes);                     // select which operation from file_operations_table
    return ret_val;
}

/* Function Name: system_getargs(uint8_t* buf, int32_t nbytes)
*   INPUTS: buffer, number of bytes
*   OUTPUT: 0 if successful; -1 if fail
*   NOTES:  - copies the command line arguments into the buffer
*           - returns 0 if successful
*           - returns -1 if fail
*/
int32_t system_getargs(uint8_t* buf, int32_t nbytes){

    if(buf == NULL){ //check if buffer is null
        return -1;
    }
    if(pcb_obj->args[0] == '\0'){ //check if there are any arguments
        return -1;
    }
    //printf("system_getargs: argument: %s \n", pcb_obj->args);
    strncpy((int8_t*)buf, (int8_t*)(pcb_obj->args), nbytes); //copy arguments into buffer
    return 0;
}

/* Function Name: system_vidmap(uint8_t** screen_start)
*   INPUTS: screen_start  - pointer to the start of video memory
*   OUTPUT: 0 if successful; -1 if fail
*   NOTES:  - maps the text-mode video memory into user space at a pre-set virtual address
*           - returns 0 if successful
*           - returns -1 if fail
*/
int32_t system_vidmap(uint8_t** screen_start){ 
    //check if screen_start is null
    if(screen_start == NULL){
        // printf("system_vidmap: screen_start is NULL \n");
        return -1;
    }
    //check if screen_start is in the correct range
    if((uint32_t)screen_start < IMG_BIG_START || (uint32_t)screen_start > (IMG_BIG_START + PAGESIZE_4MB)){
        // printf("system_vidmap: screen_start is out of range \n");
        return -1;
    }
    
    *screen_start = (uint8_t*) USER_VMEM_ADDR; //set screen_start to the correct address

    map_vidmem(); //map video memory and flush TLB
    
    return 0;
}

/*HELPER FUNCTIONS*/ 

/*Returns address for top of kernel stack for the given pid, 
which also corresponds to the given pid's kernel stack*/
/*Function name: find_PCB(int32_t pid)
*   INPUTS: pid
*   OUTPUT: address of PCB struct for given pid
*   NOTES:  - returns address of PCB struct for given pid
*/
int32_t find_PCB(int32_t pid){
    return END_OF_KERNEL_PAGE - (KERNEL_STACK_SIZE *(pid + 1)); 
}


/*Checks the PCB structs to see if they are active for both PIDs*/
/*Function name: assign_pid()
*   INPUTS: none
*   OUTPUT: pid if successful; -1 if fail
*   NOTES:  - returns pid if successful
*           - returns ASSIGN_PID_ERROR if fail
*/
int32_t assign_PID(){
    int i; 
    for (i = 0; i < MAX_PROCESSES; i++) {   
        if (((pcb_t *)find_PCB(i))->active == 0) { 
            return i;
        }
    }
    printf("assign_PID: Maximum processes running!\n"); 
    return ASSIGN_PID_ERROR;    
}
