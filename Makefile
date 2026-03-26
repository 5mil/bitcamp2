CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -pthread
LDFLAGS = -lpthread -lncurses -lm

OBJS = bitcamp_core.o sha256.o sha256t.o platform.o miner.o ui.o

all: bitcamp

bitcamp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o bitcamp

.PHONY: all clean
