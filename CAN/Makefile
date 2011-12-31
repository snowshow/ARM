EXEC = bin/emitter bin/converter

CC = gcc
LD = gcc
MAKE = make

CFLAGS = -std=gnu99 -Iinclude
LDFLAGS = -lrt -lcan -Llib 
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = 
#-O3

all: sharedlib $(EXEC)

sharedlib:
	@make -C lib

bin/%: src/%.c
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	@rm -f $(OBJ)

mrproper: clean
	@rm -f bin/$(EXEC)

rebuild: mrproper all

librebuild: mrproper
	@make -C lib mrproper
	@make all

install-lib: sharedlib
	@make -C lib install

install: install-lib
	cp bin/* /usr/local/bin

.PHONY: clean mrproper
