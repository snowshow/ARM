LIB = libcan.so

SRCDIR = src
OBJDIR = obj
LIBDIR = .

CC = gcc
LD = gcc

CFLAGS = -std=gnu99 -Iinclude -fPIC
LDFLAGS = -shared
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

SRC = $(wildcard src/*.c)
OBJ = $(subst $(SRCDIR), $(OBJDIR), $(SRC:.c=.o))

all: $(LIBDIR) $(OBJDIR) $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) -o $@ -c $^

$(LIB): $(OBJ)
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(LDFLAGS) -o $@ $^

$(LIBDIR):
	@mkdir -p $(LIBDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@rm -f $(OBJ)

mrproper: clean
	@rm -f $(LIB)

rebuild: mrproper all

.PHONY: clean mrproper
