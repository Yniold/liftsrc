#
# Makefile for ELectronic Konverter test tool 
#

VERSION = 0.1
PROJECT = elekStatus

CC=gcc
SHELL=/bin/sh

modules=../lib/udptools.o

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o
 
elekStatus : elekStatus.c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CFLAGS) elekStatus.c $(modules) -o elekStatus -lrt -ggdb
	     cp elekStatus $(INSTDIR)

