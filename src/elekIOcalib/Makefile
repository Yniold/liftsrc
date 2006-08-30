#
# Makefile for ELectronic Konverter test tool
#

VERSION=0.1
PROJECT=elekIOcalib

SHELL=/bin/sh

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC -Wa,-mfpu=softvfp
LDFLAGS :=-lrt
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o serial.o

headers=elekIOcalib.h ../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

