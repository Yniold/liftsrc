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

elekIO : elekIO.c ../include/elekIO.h
	$(CC)  $(CPPFLAGS) $(CFLAGS) -c elekIO.c $(LDFLAGS)
	cp elekIO.o ../lib
 
