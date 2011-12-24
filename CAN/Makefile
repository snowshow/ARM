EXEC = bin/emitter bin/decoder

CC = gcc
LD = gcc
MAKE = make

CFLAGS = -std=gnu99 -Iinclude
LDFLAGS = -lrt -lcan -Llib 
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

all: sharedlib $(EXEC)

sharedlib:
	make -C lib

bin/%: src/%.c
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f bin/$(EXEC)

rebuild: mrproper all

.PHONY: clean mrproper
