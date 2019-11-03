# File name:        Makefile
# Description:      Makefile script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

PROJECTNAME := pecka

CC = avr-gcc
#CCFLAGS = -mmcu=atmega328p
CCFLAGS = -mmcu=atmega32u4 -O2 -Wall 
#CCFLAGS = -mmcu=atmega2560
MKDIR_P = mkdir -p

LIBS =

TOPDIR := $(CURDIR)
SRCDIR := $(TOPDIR)/src
INCDIR := $(TOPDIR)/include
BINDIR := $(TOPDIR)/build

OUTDIR := $(BINDIR) $(BINDIR)/driver

SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRCS))

all: directories program

program: $(OBJS)
	$(CC) $(CCFLAGS) -o $(BINDIR)/$(PROJECTNAME).elf $^ $(LIBS) 

directories:
	$(shell mkdir -p $(OUTDIR))

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -o $@ -c $< -I $(INCDIR)

clean:
	rm -Rf build/*
