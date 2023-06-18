#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "i8259.h"

//function declarations
void rtc_init();
void rtc_handler();

/* set the interrupt frequency to freq, by a power of 2 no larger than 1024 */
int rtc_set_freq(int freq);

int rtc_write(int32_t file_index, const void *buf, int32_t nbytes);

int32_t rtc_read(int32_t file_index, void* buf, int32_t nbytes);

int rtc_open();

int32_t rtc_close(int32_t file_index);

#endif
