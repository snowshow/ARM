EXEC = ia

CPP = gcc
LD = gcc

CPPFLAGS = -Iinclude
LDFLAGS = 
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

SRC = $(wildcard src/*.cpp)
OBJ = $(subst src, obj, $(SRC:.cpp=.o))

all: $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $(DEBUG) $(WARNING) $(LDFLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CPP) $(CFLAGS) $(OPTIMIZE) $(DEBUG) $(WARNING) -o $@ -c $^

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f $(EXEC)
	rm -f gmon.out

install: $(EXEC)
	cp $(EXEC) /usr/local/bin/

rebuild: mrproper all

.PHONY: clean mrproper
