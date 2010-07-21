#
# Makefile for ELectronic Konverter test tool 
#

CC=gcc
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


.SUFFIXES:
.SUFFIXES: .c .o

cfgnear : cfgnear.c ../include/cfgopts.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c cfgnear.c $(LDFLAGS)
	cp cfgnear.o ../lib
 
