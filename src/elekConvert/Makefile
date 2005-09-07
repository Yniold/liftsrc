#
# Makefile for GPS NMEA Converter
#

VERSION = 0.1
PROJECT = elekConverter

SHELL=/bin/sh

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
LDFLAGS :=-lrt
LDFLAGS +=-lcurses
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb
CFLAGS+=-fpack-struct

modules=../lib/udptools.o
modules+=../lib/getch.o

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o
 
elekConverter : elekConverter.c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) elekConverter.c $(modules) -o elekConverter $(LDFLAGS)
	     cp elekConverter $(INSTDIR)

clean: 
	rm -f ./elekConverter
	
all: clean elekConverter