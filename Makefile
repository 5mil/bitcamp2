# Compiler
CC ?= gcc

# Common flags
CFLAGS  += -Wall -Wextra -O2 -std=c11
LDFLAGS +=
LDLIBS  += -lm

# Source files
SRCS = bitcamp_core.c sha256.c sha256t.c platform.c miner.c ui.c
OBJS = $(SRCS:.c=.o)

# Platform-specific
ifeq ($(OS),Windows_NT)
    # MSYS2 MinGW64 + raylib
    CFLAGS  += -I/mingw64/include -pthread
    LDFLAGS += -L/mingw64/lib

    # IMPORTANT: rely on raylib's own dependency chain (no -lglfw3 here)
    LDLIBS  += -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lole32 -lpthread
else
    # POSIX (Linux/macOS) with system raylib
    CFLAGS  += -pthread
    LDLIBS  += -lraylib -lpthread
endif

# Default target
all: bitcamp

bitcamp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) bitcamp bitcamp.exe

.PHONY: all clean
