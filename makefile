# Makefile

IDIR = include
CC = gcc
CFLAGS = -I$(IDIR) -O0 -lblas -lpthread -lm -Wall  -g

SDIR = src
ODIR = obj

_SOURCES = gpt_main.c gpt_IO.c
SOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_OBJECTS = $(_SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))

_DEPS = gpt_defs.h gpt_IO.h uthash.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

EXE = gpt

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
