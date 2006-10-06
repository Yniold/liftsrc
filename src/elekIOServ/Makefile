#
# Makefile for ELectronic Konverter test tool
#

VERSION=0.1
PROJECT=elekIOServ

SHELL=/bin/sh

INSTDIR?=../bin

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
LDFLAGS :=-lpthread
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC -Wall
LDFLAGS :=-lrt -lpthread
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o NMEAParser.o serial.o butterfly.o

headers=elekIOServ.h ../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o

all: 	   butterflytest elekIOServ

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

butterflytest: butterflytest.c serial.c butterfly.c
	     $(CC) $(CPPFLAGS) $(CFLAGS) butterflytest.c serial.c butterfly.c -o butterflytest -lpthread
