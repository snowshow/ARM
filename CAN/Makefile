EXEC = bin/emitter bin/decoder
LIB = can

CC = gcc
LD = gcc

CFLAGS = -std=gnu99 -Iinclude
LDFLAGS = -lrt -lcan -Llib 
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

all: $(EXEC)

bin/%: src/%.c
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) $(LDFLAGS) -o $@ $^

#%.o: %.c
#	$(CC) $(CFLAGS) $(OPTIMIZE) $(DEBUG) $(WARNING) -o $@ -c $^

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f bin/$(EXEC)

rebuild: mrproper all

.PHONY: clean mrproper
