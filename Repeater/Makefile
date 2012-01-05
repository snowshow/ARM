EXEC = repeater

CC = gcc
LD = gcc

CFLAGS = -std=gnu99 -Iinclude
LDFLAGS = -lrt
DEBUG = 
WARNING = -Wall -pedantic
OPTIMIZE = -O3

SRC = $(wildcard src/*.c)
OBJ = $(subst src, obj, $(SRC:.c=.o))

all: bin obj $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $(DEBUG) $(WARNING) $(LDFLAGS) -o $@ $^

obj:
	@mkdir obj

bin:
	@mkdir bin

obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $(DEBUG) $(WARNING) -o $@ -c $^

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f $(EXEC)
	rm -f gmon.out

install: $(EXEC)
	cp $(EXEC) /usr/local/bin/

rebuild: mrproper all

.PHONY: clean mrproper
