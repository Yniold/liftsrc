#
# Makefile for ELectronic Konverter test tool 
#

VERSION=0.1
PROJECT=elekIOServ

CC=gcc
SHELL=/bin/sh

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o

headers=elekIOServ.h ../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o
 
$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) -lrt -ggdb
	     cp $(PROJECT) $(INSTDIR)

