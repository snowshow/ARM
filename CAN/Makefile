#EXEC = bin/emitter bin/converter
SRC = $(wildcard src/*.c)
EXEC = $(subst src, bin, $(SRC:.c=))

CC = gcc
LD = gcc
MAKE = make

CFLAGS = -std=gnu99 -Iinclude
LDFLAGS = -Llibcan -lcan -lpthread
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = 
#-O3

all: sharedlib bindir $(EXEC)

sharedlib:
	@make -C libcan

bindir:
	@if [ ! -d bin ]; then mkdir bin; fi

bin/%: src/%.c
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	@rm -f bin/*

mrproper: clean

rebuild: mrproper all

librebuild: mrproper
	@make -C libcan mrproper
	@make all

install-lib:
	@make -C libcan install

install:
	cp bin/* /usr/local/bin

.PHONY: clean mrproper
