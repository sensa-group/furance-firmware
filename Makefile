# File name:        Makefile
# Description:      Makefile script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

PROJECTNAME := pecka

CC = avr-gcc
CCFLAGS = -mmcu=atmega328p

LIBS =

TOPDIR := $(CURDIR)
SRCDIR := $(TOPDIR)/src
INCDIR := $(TOPDIR)/include
BINDIR := $(TOPDIR)/build

SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRCS))

all: $(OBJS)
	$(CC) $(CCFLAGS) -o $(BINDIR)/$(PROJECTNAME).elf $^ $(LIBS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -o $@ -c $<

clean:
	rm -Rf build/*
