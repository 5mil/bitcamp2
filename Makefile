# Compiler (can be overridden by environment)
CC ?= gcc

# Base compiler flags
CFLAGS += -Wall -Wextra -O2 -std=c11
LDFLAGS +=
LDLIBS  += -lm

# Platform-specific configurations
ifeq ($(OS),Windows_NT)
    # Windows + PDCurses (via MSYS2)
    # Add -pthread for the miner thread
    CFLAGS  += -I/mingw64/include -I/mingw64/include/pdcurses -pthread
    
    # Force fully static executable so it runs anywhere without DLLs
    LDFLAGS += -static -static-libgcc -static-libstdc++
    
    # Link pdcurses and pthread statically
    LDLIBS  += -Wl,-Bstatic -lpdcurses -lpthread
else
    # POSIX (Linux/macOS) with ncurses + pthread
    CFLAGS  += -pthread
    LDLIBS  += -lpthread -lncurses
endif

# Object files
OBJS = bitcamp_core.o sha256.o sha256t.o platform.o miner.o ui.o

# Default target
all: bitcamp

# Link the final executable
bitcamp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up build artifacts
clean:
	rm -f *.o bitcamp bitcamp.exe

.PHONY: all clean
