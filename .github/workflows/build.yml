# Compiler (can be overridden by environment)
CC ?= gcc

# Base compiler flags
CFLAGS += -Wall -Wextra -O2 -std=c11
LDFLAGS +=
LDLIBS  += -lm

# Platform-specific configurations
ifeq ($(OS),Windows_NT)
    # Windows + Raylib
    CFLAGS  += -I/mingw64/include -pthread
    
    # Force fully static executable
    LDFLAGS += -static -static-libgcc -static-libstdc++
    
    # Link Raylib and Windows system libraries statically
    LDLIBS  += -Wl,-Bstatic -lraylib -lpthread \
               -Wl,-Bdynamic -lgdi32 -lwinmm -lopengl32 -lcomdlg32 -lole32
else
    # POSIX (Linux/macOS) fallback
    CFLAGS  += -pthread
    LDLIBS  += -lpthread -lm
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
