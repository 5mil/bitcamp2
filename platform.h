#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

typedef void *(*platform_thread_fn)(void *);

int      platform_thread_start(void **handle, platform_thread_fn fn, void *arg);
void     platform_sleep_ms(uint32_t ms);
uint64_t platform_now_ms(void);
void     platform_log(const char *fmt, ...);

#endif
