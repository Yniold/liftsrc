#
# Makefile for ELectronic Konverter test tool
#
#

VERSION=0.1
PROJECT=elekIOGSB_

SHELL=/bin/sh

INSTDIR?=../bin

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
LDFLAGS :=-lrt -lm
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
LDFLAGS :=-lrt -lm
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb

modules=../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o

headers=elekIOGSB.h ../include/elekIO.h ../include/elekIOPorts.h

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) -std=gnu99 $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)
	     
cgigateway : cgigateway.c ../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o cgigateway.h ../include/elekIO.h ../include/elekIOPorts.h
	     echo CGI-Gateway
	     $(CC) $(CPPFLAGS) -std=gnu99 $(CFLAGS) cgigateway.c ../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o -o cgigateway.cgi $(LDFLAGS)
	     cp cgigateway.cgi $(INSTDIR)

