#
# Makefile for ELectronic Konverter test tool 
#

VERSION = 0.1
PROJECT=showq

CC=gcc
SHELL=/bin/sh

modules=../lib/udptools.o 

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o
 
$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(PROJECT).c $(modules) -o $(PROJECT) -lrt -ggdb
	     cp $(PROJECT) $(INSTDIR)

