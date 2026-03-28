# Compiler (can be overridden by environment)
CC ?= gcc

# Base flags (we append to these from the workflow env)
CFLAGS += -Wall -Wextra -O2 -std=c11
LDFLAGS +=
LDLIBS  += -lm

# Platform‑specific bits
ifeq ($(OS),Windows_NT)
    # Windows + PDCurses (via MSYS2 mingw-w64-x86_64-pdcurses)
    CFLAGS  += -I/mingw64/include -I/mingw64/include/pdcurses
    LDLIBS  += -lpdcurses
else
    # POSIX (Linux/macOS) with ncurses + pthread
    CFLAGS  += -pthread
    LDLIBS  += -lpthread -lncurses
endif

OBJS = bitcamp_core.o sha256.o sha256t.o platform.o miner.o ui.o

all: bitcamp

bitcamp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o bitcamp

.PHONY: all clean
