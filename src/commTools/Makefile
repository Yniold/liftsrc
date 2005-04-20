#
# Makefile for ELectronic Konverter test tool
#

SHELL=/bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

all:

#udptools : udptools.c ../include/elekIOPorts.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c udptools.c   -lrt -ggdb
	cp udptools.o ../lib

#bitop:	bitop.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c bitop.c   -lrt -ggdb
	cp bitop.o ../lib
