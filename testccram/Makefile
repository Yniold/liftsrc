#
# Makefile for ELectronic Konverter test tool
#

VERSION=0.1
PROJECT=testccram

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

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o

headers=../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o

all: 	   testccram

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)