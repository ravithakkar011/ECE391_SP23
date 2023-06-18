#ifndef TESTS_H
#define TESTS_H

#include "x86_desc.h"
#include "lib.h"
#include "IDT.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal_driver.h"
#include "filesystem.h"

int idt_test(void);

int idt_full_test(void);

int paging_test(void);

int divide_zero_test(void);

//int systemcall_test(void)
int test_rtc_driver(int32_t feq_test);
/* test launcher*/ 
void launch_tests();
int32_t test_read_dentry_by_index(void);
int test_read_dentry_by_name(void);
int32_t test_dentry_search(void);
int test_read_data(void);
int filesystem_test(void);
int test_directory_open(void);
int test_directory_close(void);
int test_directory_write(void);
int test_read_directory(void);

#endif /* TESTS_H */
