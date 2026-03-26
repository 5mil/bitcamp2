#include "platform.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

int platform_thread_start(void **handle, platform_thread_fn fn, void *arg) {
    pthread_t *t = (pthread_t *)handle;
    return pthread_create(t, NULL, fn, arg);
}

void platform_sleep_ms(uint32_t ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000ull;
    nanosleep(&ts, NULL);
}

uint64_t platform_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull;
}

void platform_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}
