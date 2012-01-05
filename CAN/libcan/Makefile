LIB = libcan.so

SRCDIR = src
OBJDIR = obj
LIBDIR = .

CC = gcc
LD = gcc

CFLAGS = -std=gnu99 -Iinclude -fPIC 
#-fvisibility=hidden
LDFLAGS = -shared
DEBUG = -g
WARNING = -Wall -pedantic
OPTIMIZE = -O3

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(subst $(SRCDIR), $(OBJDIR), $(SRC:.c=.o))

all: $(LIBDIR) $(OBJDIR) $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(DEBUG) $(WARNING) $(OPTIMIZE) $(CFLAGS) -o $@ -c $^

$(LIB): $(OBJ)
	$(LD) $(DEBUG) $(WARNING) $(OPTIMIZE) $(LDFLAGS) -o $@ $^

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@rm -f $(OBJ)

mrproper: clean
	@rm -f $(LIB)

rebuild: mrproper all

install: all
	cp $(LIB) /usr/lib

.PHONY: clean mrproper
