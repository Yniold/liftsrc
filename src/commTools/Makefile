#
# Makefile for ELectronic Konverter test tool
#

SHELL=/bin/sh

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS := -ggdb

.SUFFIXES:
.SUFFIXES: .c .o

all:

#udptools : udptools.c ../include/elekIOPorts.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c udptools.c $(LDFLAGS)
	cp udptools.o ../lib

#bitop:	bitop.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c bitop.c $(LDFLAGS)
	cp bitop.o ../lib
