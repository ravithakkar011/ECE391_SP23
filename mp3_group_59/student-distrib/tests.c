#define PASS 1
#define FAIL 0

#include "tests.h"

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

/*Declare functions that need to be declared*/
int dereference(int *ptr);

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* Clear Screen Test -
 * 
 * Clears screen using clear(); 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Verifys that tests are working properly
 * Coverage: test.c, kernel.c
 * Files: lib.c
 */
int clear_screen_test(void){
	TEST_HEADER;
	clear();
	while(1);
	return FAIL;
}


/* IDT Test -
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(void){
	TEST_HEADER;
	clear();
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

// add more tests here


/* Paging test -
 * 
 * Prints paging memory contents to the screen
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints to screen, clears
 * Coverage: Paging
 * Files: idt.c/h
 */
int paging_test(void) {
	clear();

	TEST_HEADER;
	uint32_t addr;
	for (addr = 0x400000; addr < 0x800000; addr += 0x100000 + 50) {
	printf("Addr: %#x\tContent: %#x\n", addr, *(uint32_t *)addr);
	}
	addr = 0x8F0000;
	return PASS;
}

/* Divide by Zero Test -
 * 
 * Checks to see if the exception is handled after a divide by zero is performed.
 * Inputs: None
 * Outputs: None
 * Side Effects: A blue screen of death should show on the screen
 * Coverage: Exception Handling
 * Files: idt.c/h
 */
int divide_zero_test(void){
	TEST_HEADER;
	clear();
	int i = 1;
	int j = 0;
	int k = 0;
	k = i / j; // dividing 1 / 0

	return FAIL; // if the exception is successfully handled, then this line should not be reached.
}

/* Invalid Opcode Test -
 * 
 * Checks to see if the exception (6) is handled after an invalid opcode is sent.
 * Inputs: None
 * Outputs: None
 * Side Effects: A blue screen of death should show on the screen
 * Coverage: Exception Handling
 * Files: idt.c/h
 */
int invalid_opcode_test(void){
	TEST_HEADER;
	clear();
	asm volatile("ud2");

	return FAIL; // if the exception is successfully handled, then this line should not be reached.
}

 /* System Call Test -
 * 
 * Tests if system calls can successfully be called.
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints that a system call was called
 * Coverage: IDT, System Calls (Useful for future checkpoints)
 * Files: idt.c/idt.h, syscall.c/syscall.h
 */
 int systemcall_test(void){
	TEST_HEADER;
	clear();
	__asm__("int	$0x80"); //0x80 is the vector number for system calls
	return FAIL;

 }

 /* Dereference NULL test -
 * 
 * Tests if an exception occurs if NULL is dereferenced
 * Inputs: None
 * Outputs: None
 * Side Effects: BSOD
 * Coverage: IDT, Exception Handling, Paging
 * Files: idt.c/h
 */

 int deref_NULL_test(void){
	TEST_HEADER;
	clear();
	dereference(NULL);
	
	return FAIL; // never reaches here if exception is called

 }

  /*  dereference(int *ptr)
 * Dereferences a pointer for the dereference NULL test
 * Inputs: None
 * Outputs: dereferenced pointer
 * Side Effects: causes page exception 0_0
 */
int dereference(int *ptr) {
    return *ptr;
}

/* Checkpoint 2 tests */

 /* RTC Driver Test -
 * 
 * Tests if RTC driver is properly working based on an input frequency
 * Inputs: test frequency
 * Outputs: None
 * Side Effects: Prints a character at every rtc interrupt to the screen
 * Coverage: RTC driver
 * Files: rtc.c/rtc.h
 */
int test_rtc_driver(int32_t feq_test){
	clear();
	set_cursor(0, 0);
	TEST_HEADER;
	int i =0;
	if(rtc_write(NULL, &feq_test, 4)!= 0){
		printf("Invalid Frequency!!!\n");
		return FAIL;
	}
	for ( i = 0; i < 20; i++)
	{	rtc_read(NULL, NULL, 0);
		putc('0');
	}
	clear();
	set_cursor(0, 0);
	feq_test =feq_test*2;
	if(rtc_write(NULL, &feq_test, 4)!= 0){
		printf("Invalid Frequency!!!\n");
		return FAIL;
	}
	for ( i = 0; i < 20; i++)
	{	rtc_read(NULL, NULL, 0);
		putc('0');
	}
	clear();
	set_cursor(0, 0);
	feq_test =feq_test*4;
	if(rtc_write(NULL, &feq_test, 4)!= 0){
		printf("Invalid Frequency!!!\n");
		return FAIL;
	}
	for ( i = 0; i < 30; i++)
	{	rtc_read(NULL, NULL, 0);
		putc('0');
	}


	return PASS;
}


 /* Terminal Read/Write Test -
 * 
 * Tests if terminal_read and terminal_write drivers are properly working
 * Inputs: None
 * Outputs: None
 * Side Effects: Echos a line from the keyboard buffer.
 * Coverage: Terminal driver
 * Files: terminal_driver.c/h, keyboard.c/h
 */
int terminal_driver_test(void){
	clear();
	set_cursor(0,0);

	TEST_HEADER;

	int check_terminal_open;
	int check_terminal_close;
	char filename[BYTES_32B] = {"frame0.txt"};
	check_terminal_open = terminal_open((uint8_t*)filename);
	//printf("%d", check_terminal_open);
	if (check_terminal_open != 0){
		return FAIL;
	}

	int index = 0;
	int nbytes_ret_check;
	keyboard_buffer[index++] = 'h';
	keyboard_buffer[index++] = 'e';
	keyboard_buffer[index++] = 'l';
	keyboard_buffer[index++] = 'l';
	keyboard_buffer[index++] = 'o';
	keyboard_buffer[NUM_CHARS_KB] = '\n';

	uint8_t buffer[NUM_CHARS_KB];

	nbytes_ret_check = terminal_read(0, buffer, NUM_CHARS_KB);
	if (nbytes_ret_check == 0){
		return FAIL;
	}
	nbytes_ret_check = terminal_write(0, buffer, NUM_CHARS_KB);
	if (nbytes_ret_check == 0){
		return FAIL;
	}

	check_terminal_close = terminal_close(0);
	//printf("%d", check_terminal_close);
	if (check_terminal_close != 0){
		return FAIL;
	}

	return PASS;
 }



/*--------------File System Tests--------------*/
int32_t test_read_dentry_by_index(void) {
	clear();
	TEST_HEADER;

	int index_bootblock = 3;
	dentry_t test_dentry_input = boot_block->direntries[index_bootblock];
	dentry_t test_dentry_output;
	if(read_dentry_by_index(index_bootblock, &test_dentry_output) == -1){return FAIL;}
	// printf("%d \n", test_dentry_output.inode_num);
	// printf("%d \n", test_dentry_input.inode_num);
	if((int)test_dentry_output.inode_num == test_dentry_input.inode_num){return PASS;}
	else{return FAIL;}
}

int32_t test_read_dentry_by_name(void) {
	clear();
	TEST_HEADER;
	int index_bootblock = 3;
	char file_name[BYTES_32B];
	dentry_t test_dentry_input = boot_block->direntries[index_bootblock];
	strncpy((int8_t*)file_name, (int8_t*)test_dentry_input.file_name, BITS_32);

	dentry_t test_dentry_output;
	read_dentry_by_name((uint8_t*)file_name, &test_dentry_output);
	// printf("%s \n", test_dentry_input.file_name);
	// printf("%s \n", test_dentry_output.file_name);
	if(!(strncmp((char*)file_name, (char*)(test_dentry_output.file_name), BITS_32))){return PASS;}
	else{return FAIL;}
}

/* tests both read by index & read by name */
int32_t test_dentry_search(void){
	clear();
	TEST_HEADER;

	dentry_t access_by_name;
	dentry_t access_by_index;

	int index_bootblock = 3;
	read_dentry_by_index(index_bootblock, &access_by_index);
	if(read_dentry_by_name((uint8_t*)access_by_index.file_name, &access_by_name) == -1){return FAIL;}
	// printf("%s \n", access_by_index.file_name);
	// printf("%d \n", access_by_name.inode_num);
	if (!(strncmp(access_by_index.file_name, access_by_name.file_name, BYTES_32B))){return PASS;}
	else{
		printf("Filenames don't match up");
		return FAIL;
	}
}

/* test prints whole file according to offset */
int test_read_data(void) {
	clear();
	set_cursor(0,0);
	TEST_HEADER;
	int i = 0;

	dentry_t test_dentry;	
	char file_name[BYTES_32B] = "counter";				
	read_dentry_by_name((uint8_t*)file_name, &test_dentry);	// inode = 10 =>frame0.txt (small file)
	uint32_t inode_index = test_dentry.inode_num;

	// printf("%s \n", test_dentry.file_name);
	// printf("%d \n", inodes[test_inode_num].length);
	int offset = 0;
	int32_t file_length = inodes[inode_index].length;
	char buff[file_length];																	// size of one block
	int32_t num_bytes_copied = read_data(test_dentry.inode_num, offset, (uint8_t*)buff, file_length-offset);			// 187 is file size according to inodes[test_inode_num].length

	/* test just first block */
	// int32_t first_data_block_idx = inodes[test_dentry.inode_num].data_block_num[0];
    // printf("%d \n", first_data_block_idx);
    // memcpy(buff, data_blocks[first_data_block_idx].data, BYTES_4KB);
	if(num_bytes_copied == -1){return FAIL;}
	else{
		// printf("%s \n", buff);
		for (i = 0; i < file_length; ++i) printf("%x", buff[i]);
		//printf("%d \n", num_bytes_copied);
		return PASS;
	}
}

/* test filesystem w/ terminal driver */
int filesystem_test(void){
	clear();
	TEST_HEADER;
	dentry_t dentry_name;
	dentry_t dentry_index;
	int i;
	for(i = 0; i < boot_block->dir_count; i++){
		read_dentry_by_index(i, &dentry_index);
		read_dentry_by_name((uint8_t*)dentry_index.file_name, &dentry_name);
		int cmp_ret = strncmp((int8_t*)dentry_name.file_name, (int8_t*)dentry_index.file_name, BYTES_32B);

		if(cmp_ret != 0){
			printf("Strings don't match up");
			return FAIL;
		}
		//printf("%s\n", dentry_name.file_name);
		terminal_write(0, dentry_name.file_name, BYTES_32B);
	}
	return PASS;
}

/* test filesystem open, close, & write functions */
int test_directory_open(){
	clear();
	TEST_HEADER;

	char filename[BYTES_32B] = {"grep"};	
	if (!directory_open((uint8_t*)filename)){return PASS;}
	return FAIL;
}

int test_directory_close(){
	clear();
	TEST_HEADER;

	if (!directory_close(0)){return PASS;}
	return FAIL;
}

int test_directory_write(){
	clear();
	TEST_HEADER;

	char filename[BYTES_32B] = "grep";
	if (!directory_write(0, (uint8_t*)filename, 10)){return PASS;}
	return FAIL;
}

//read all directory and show them in screen
int test_read_directory(void){
	// clear();
	// TEST_HEADER;
	// uint8_t buf[4096]; //4kb
	// buf[32] = '\0';
	// int i;
	// for (i = 0; i < 63; i++){// 63 is the max directory number in filesystem
	// 	if (read_directory(buf, i) == -1)
	// 		break;
	// 	printf((int8_t*)buf);
	// 	printf("\n");
	// }
	return PASS;
}

/* Checkpoint 3 tests */

 /* System Call Using registers -
 * 
 * Tests if the system call handler is correctly calling the halt system call
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints that a system call was called
 * Coverage:  ::TODO::
 * Files: idt.c/idt.h, syscall.c/syscall.h
 */
 int systemcall_register_test(void){
	TEST_HEADER;
	clear();
	__asm__("movl	$5, %eax"); //0x80 is the vector number for system calls
	__asm__("int	$0x80"); 	//0x80 is the vector number for system calls
	return FAIL;

 }


/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here

	//checkpoint 1 tests

	//TEST_OUTPUT("divide_zero_test", divide_zero_test());
	//TEST_OUTPUT("systemcall_test", systemcall_test());
	//TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	//TEST_OUTPUT("deref_NULL_test", deref_NULL_test());
	//TEST_OUTPUT("paging_test", paging_test());
	
	// checkpoint 2 tests

	//TEST_OUTPUT("test_rtc_driver", test_rtc_driver(2));
	//TEST_OUTPUT("terminal_driver_test", terminal_driver_test());
	//TEST_OUTPUT("test_read_dentry_by_index", test_read_dentry_by_index());
	//TEST_OUTPUT("test_read_dentry_by_name", test_read_dentry_by_name());
	//TEST_OUTPUT("test_dentry_search", test_dentry_search());
	//TEST_OUTPUT("test_read_data", test_read_data());
	//TEST_OUTPUT("filesystem_test", filesystem_test());
	//TEST_OUTPUT("test_directory_open", test_directory_open());
	//TEST_OUTPUT("test_directory_close", test_directory_close());
	//TEST_OUTPUT("test_directory_write", test_directory_write());
	//TEST_OUTPUT("test_read_directory", test_read_directory());
	//TEST_OUTPUT("systemcall_register_test", systemcall_register_test());
}
