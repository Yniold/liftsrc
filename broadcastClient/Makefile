#
# Makefile for Auxiliary Data Server (Meteobox, Ship's GPS, Water Temp etc)
#
#

VERSION=0.1
PROJECT=broadcastClient

SHELL=/bin/sh

INSTDIR?=../bin

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
LDFLAGS :=-lrt -lm
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC -combine
LDFLAGS :=-lrt -lm -lncurses
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) -std=gnu99 $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

