#
# Makefile for ELectronic Konverter test tool
#
# Notes:
#
# If an environment variable named ARMBUILD is set to 1
# or passed by make
# the tools will be cross compiled for the ARM9 board
#
# Note that there will be a preprocessor variable defined
# RUNONARM if the tools are crosscompiled for ARM and
# RUNONPC if the tools are built for the i386 environment
# this will allow to use ifdefs for platform dependend code

VERSION=08
PROJECT=all

SHELL:=/bin/sh
SOURCEDIR = $(shell /bin/pwd)
DESTDIR?=$(shell /bin/pwd)/bin/
INSTDIR?=$(DESTDIR)
LIBDIR := $(DESTDIR)/lib/

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM -Wa,-mapcsfloat
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
endif

CC = $(CROSS_COMPILE)gcc
export CC

flags= -j -k

CFLAGS+=-fpack-struct

.SUFFIXES:
.SUFFIXES: .c .o
all: horus install

horus:
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) DESTDIR=$(LIBDIR) $(flags) -C cfgnear
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) DESTDIR=$(LIBDIR) $(flags) -C commTools
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) DESTDIR=$(LIBDIR) $(flags) -C elekIO

	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C eCmd
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekIOServ
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekIOcalib
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekStatus
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C statView
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C etalon
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C debugMon
#	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) DESTDIR=$(DESTDIR) $(flags) -C spectrumAnalyzer
#	$(MAKE) CC=$(CC) CFLAGS=$(CFLAGS) DESTDIR=$(DESTDIR) $(flags) -C instServer

install:
ifeq ($(ARMBUILD),1)
	@echo ARM-Install
	if [ ! -d $(SOURCEDIR)/bin ]; then mkdir $(SOURCEDIR)/bin; fi
	cp $(SOURCEDIR)/eCmd/eCmd $(DESTDIR)
	cp $(SOURCEDIR)/elekIOServ/elekIOServ $(DESTDIR)
	cp $(SOURCEDIR)/elekIOcalib/elekIOcalib $(DESTDIR)
	cp $(SOURCEDIR)/elekStatus/elekStatus $(DESTDIR)
	cp $(SOURCEDIR)/statView/statView $(DESTDIR)
	cp $(SOURCEDIR)/etalon/etalon $(DESTDIR)
	cp $(SOURCEDIR)/debugMon/debugMon $(DESTDIR)
else
	@echo LIFT-Install
	# test if ../bin is a file (left in CVS due to wrong makefile ages ago....)
	# if so, remove this file
	if [ -f ../bin ]; then rm -f ../bin; fi
	# test if ../bin is a directory, if not create a new one
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	cp $(DESTDIR)/eCmd ../bin
	cp $(DESTDIR)/elekIOServ ../bin
	cp $(DESTDIR)/elekIOcalib ../bin
	cp $(DESTDIR)/elekStatus ../bin
	cp $(DESTDIR)/statView ../bin
	cp $(DESTDIR)/etalon ../bin
	cp $(DESTDIR)/debugMon ../bin
	chmod +x $(DESTDIR)*
	chmod +x ./scripts/*
endif

clean:

	rm -f ./cfgnear/*.o
	rm -f ./commTools/*.o
	rm -f ./elekIO/*.o
	rm -f ./eCmd/eCmd
	rm -f ./bin/eCmd
	rm -f ./elekIOServ/elekIOServ
	rm -f ./elekIOServ/*.o
	rm -f ./elekIOcalib/elekIOcalib
	rm -f ./elekIOcalib/*.o
	rm -f ./bin/elekIOServ
	rm -f ./elekStatus/elekStatus
	rm -f ./bin/elekStatus
	rm -f ./statView/statView
	rm -f ./bin/statView
	rm -f ./etalon/etalon
	rm -f ./bin/etalon
	rm -f ./debugMon/debugMon
	rm -f ./bin/debugMon
	rm -f ./spectrumAnalyzer/spectrumAnalyzer
	rm -f ./spectrumAnalyzer/*.o
# test if there is a directory called 'bin' in '../' before trying to delete its contents
	if [ -d ../bin ]; then rm -f ../bin/debugMon; fi
	if [ -d ../bin ]; then rm -f ../bin/eCmd; fi
	if [ -d ../bin ]; then rm -f ../bin/elekIOServ; fi
	if [ -d ../bin ]; then rm -f ../bin/elekIOcalib; fi
	if [ -d ../bin ]; then rm -f ../bin/elekStatus; fi
	if [ -d ../bin ]; then rm -f ../bin/statView; fi
	if [ -d ../bin ]; then rm -f ../bin/etalon; fi
	
#	$(MAKE) DESTDIR=$(DESTDIR) $(flags) -C instServer
#        rm ./instServer/*.o
#       rm ./instServer/instServer
#       rm ./bin/instServer
#       rm ../bin/instServer
	rm -f ./lib/*.o

.PHONY: install clean
