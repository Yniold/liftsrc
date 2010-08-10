#
# Makefile for ELectronic Konverter test tool
#
#

VERSION=0.1
PROJECT=elekIOGSB

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

all : elekIOGSB

elekIOGSB: subbuild1 subbuild2

subbuild1 : elekIOGSB.c $(modules) $(headers)
	     @echo now building elekIOGSB $(VERSION)...
	     $(CC) $(CPPFLAGS) -std=gnu99 $(CFLAGS) elekIOGSB.c $(modules) -o elekIOGSB $(LDFLAGS)
	     cp elekIOGSB $(INSTDIR)
	     
subbuild2 : cgigateway.c ../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o cgigateway.h ../include/elekIO.h ../include/elekIOPorts.h
	     @echo now building CGI-Gateway...
	     $(CC) $(CPPFLAGS) -std=gnu99 $(CFLAGS) cgigateway.c ../lib/elekIO.o ../lib/udptools.o ../lib/bitop.o -o cgigateway.cgi $(LDFLAGS)
	     cp cgigateway.cgi $(INSTDIR)
	     
clean:		
		@echo cleaning up the mess...
		rm cgigateway.cgi
		rm elekIOGSB
		
.PHONY: elekIOGSB clean all
