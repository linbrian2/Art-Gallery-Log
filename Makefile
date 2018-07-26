CFLAGS=-g -Wall -std=gnu99 -O0
LFLAGS=-lcrypto
EXE= logappend logread

all: $(EXE)

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
CFLAGS += -DLINUX -I/usr/local/ssl/include -L/usr/local/ssl/lib
endif

logappend: logappend.o
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

logread: logread.o
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

logappend.o: logappend.c
	$(CC) -c -o $@ $(CFLAGS) $^

logread.o: logread.c
	$(CC) -c -o $@ $(CFLAGS) $^

clean:
	rm -f $(EXE)
