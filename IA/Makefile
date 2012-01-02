EXEC = ia

CPP = g++
LD = g++

CPPFLAGS = -Iinclude
LDFLAGS = 
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

SRC = $(wildcard src/*.cc)
OBJ = $(subst src, obj, $(SRC:.cc=.o))

all: $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $(DEBUG) $(WARNING) $(LDFLAGS) -o $@ $^

obj/%.o: src/%.cc
	$(CPP) $(CPPFLAGS) $(OPTIMIZE) $(DEBUG) $(WARNING) -o $@ -c $^

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f $(EXEC)
	rm -f gmon.out

install: $(EXEC)
	cp $(EXEC) /usr/local/bin/

rebuild: mrproper all

.PHONY: clean mrproper
