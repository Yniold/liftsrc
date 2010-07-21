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
# don't built cross elekIOaux
ifneq ($(ARMBUILD),1)
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekIOaux
endif
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekStatus
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C statView
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C etalon
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C mirrors
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C debugMon
# don't built cross broadcastClient and spectrometerServer
ifneq ($(ARMBUILD),1)
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C broadcastClient
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) INSTDIR=$(INSTDIR) $(flags) -C spectrumAnalyzer
#	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) INSTDIR=$(INSTDIR) $(flags) -C testccram
endif	
#	$(MAKE) CC=$(CC) CFLAGS=$(CFLAGS) DESTDIR=$(DESTDIR) $(flags) -C instServer

install:
ifeq ($(ARMBUILD),1)
	@echo ARM-Install
	if [ ! -d $(SOURCEDIR)/bin ]; then mkdir $(SOURCEDIR)/bin; fi
	if [ -f $(DESTDIR) ]; then rmdir $(DESTDIR); fi
	if [ ! -d $(DESTDIR) ]; then mkdir $(DESTDIR); fi
	cp $(SOURCEDIR)/eCmd/eCmd $(DESTDIR)
	cp $(SOURCEDIR)/elekIOServ/elekIOServ $(DESTDIR)
	cp $(SOURCEDIR)/elekIOcalib/elekIOcalib $(DESTDIR)
	cp $(SOURCEDIR)/elekStatus/elekStatus $(DESTDIR)
	cp $(SOURCEDIR)/statView/statView $(DESTDIR)
	cp $(SOURCEDIR)/etalon/etalon $(DESTDIR)
	cp $(SOURCEDIR)/mirrors/mirrors $(DESTDIR)
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
	cp $(DESTDIR)/mirrors ../bin
	cp $(DESTDIR)/debugMon ../bin
	cp $(DESTDIR)/spectrometerServer ../bin
	cp $(DESTDIR)/testccram ../bin
	chmod +x $(DESTDIR)*
	chmod +x ./scripts/*
endif

clean:

	rm -f ./cfgnear/*.o
	rm -f ./commTools/*.o
	rm -f ./elekIO/*.o
	rm -f ./eCmd/eCmd
	rm -f ./bin/eCmd
	rm -f ./bin/eRead
	rm -f ./elekIOServ/elekIOServ
	rm -f ./elekIOServ/*.o
	rm -f ./bin/elekIOcalib
	rm -f ./elekIOcalib/elekIOcalib
	rm -f ./elekIOcalib/*.o
	rm -f ./bin/elekIOaux
	rm -f ./elekIOaux/elekIOaux
	rm -f ./elekIOaux/*.o
	rm -f ./bin/elekIOServ
	rm -f ./elekStatus/elekStatus
	rm -f ./bin/elekStatus
	rm -f ./statView/statView
	rm -f ./bin/statView
	rm -f ./etalon/etalon
	rm -f ./bin/etalon
	rm -f ./mirrors/mirrors
	rm -f ./bin/mirrors
	rm -f ./debugMon/debugMon
	rm -f ./bin/debugMon
	rm -f ./bin/spectrometerServer
	rm -f ./spectrumAnalyzer/spectrometerServer
	rm -f ./spectrumAnalyzer/*.o
	rm -f ./bin/broadcastClient
	rm -f ./broadcastClient/broadcastClient
	rm -f ./broadcastClient/*.o
	rm -f ./testccram/testccram
	rm -f ./testccram/*.o
	
# test if there is a directory called 'bin' in '../' before trying to delete its contents
	if [ -d ../bin ]; then rm -f ../bin/debugMon; fi
	if [ -d ../bin ]; then rm -f ../bin/eCmd; fi
	if [ -d ../bin ]; then rm -f ../bin/elekIOServ; fi
	if [ -d ../bin ]; then rm -f ../bin/elekIOcalib; fi
	if [ -d ../bin ]; then rm -f ../bin/elekStatus; fi
	if [ -d ../bin ]; then rm -f ../bin/statView; fi
	if [ -d ../bin ]; then rm -f ../bin/etalon; fi
	if [ -d ../bin ]; then rm -f ../bin/mirrors; fi
	if [ -d ../bin ]; then rm -f ../bin/spectrometerServer; fi
	if [ -d ../bin ]; then rm -f ../bin/testccram; fi
	
#	$(MAKE) DESTDIR=$(DESTDIR) $(flags) -C instServer
#        rm ./instServer/*.o
#       rm ./instServer/instServer
#       rm ./bin/instServer
#       rm ../bin/instServer
	rm -f ./lib/*.o

.PHONY: install clean
