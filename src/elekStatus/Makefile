#
# Makefile for ELectronic Konverter test tool
#

VERSION = 0.1
PROJECT = elekStatus

SHELL=/bin/sh

INSTDIR?=../bin

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

modules=../lib/udptools.o
modules+=../lib/getch.o

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o
 
elekStatus : elekStatus.c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) elekStatus.c $(modules) -o elekStatus $(LDFLAGS)
	     cp elekStatus $(INSTDIR)

