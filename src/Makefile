#
# Makefile for ELectronic Konverter test tool
#
# Notes:
#
# If an environment variable named ARMBUILD is set to 1
# the tools will be cross compiled for the ARM9 board
#
# Note that there will be a preprocessor variable defined
# RUNONARM if the tools are crosscompiled for ARM and
# RUNONPC if the tools are built for the i386 environment
# this will allow to use ifdefs for platform dependend code

VERSION=08
PROJECT=all

SHELL:=/bin/sh
INSTDIR:=$(shell /bin/pwd)/bin/
LIBDIR:=$(shell /bin/pwd)/lib/

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
endif

CC := $(CROSS_COMPILE)gcc
export CC

flags= -j -k

CFLAGS+=-fpack-struct

#modules=lib/elekIO.o lib/udptools.o
#headers=include/elekIO.h include/elekIOPorts.h

modules=
headers=

.SUFFIXES:
.SUFFIXES: .c .o

all : $(modules) $(headers)

	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C cfgnear
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C commTools
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C elekIO

	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C eCmd
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekIOServ
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekStatus
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C statView
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C etalon
	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C debugMon
#	$(MAKE) CC=$(CC) CPPFLAGS=$(CPPFLAGS) INSTDIR=$(INSTDIR) $(flags) -C spectrumAnalyzer
#	$(MAKE) CC=$(CC) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C instServer

	chmod +x $(INSTDIR)* 
	chmod +x ./scripts/*
#	cp $(INSTDIR)* ../bin
	cp $(INSTDIR)eCmd ../bin
	cp $(INSTDIR)elekIOServ ../bin
	cp $(INSTDIR)elekStatus ../bin
	cp $(INSTDIR)statView ../bin
	cp $(INSTDIR)etalon ../bin
	cp $(INSTDIR)debugMon ../bin
#	cp $(INSTDIR)instServer ../bin
#	cp ../bin/* /tmp/lift
#	tar -C ../ -czf /mnt/magera/HorusV$(VERSION).tgz src

clean:

	rm -f ./cfgnear/*.o
	rm -f ./commTools/*.o
	rm -f ./elekIO/*.o
	rm -f ./eCmd/eCmd
	rm -f ./bin/eCmd
	rm -f ./elekIOServ/elekIOServ
	rm -f ./elekIOServ/*.o
	rm -f ./bin/elekIOServ
	rm -f ./elekStatus/elekStatus
	rm -f ./bin/elekStatus
	rm -f ./statView/statView
	rm -f ./bin/statView
	rm -f ./etalon/etalon
	rm -f ./bin/etalon
	rm -f ./debugMon/debugMon
	rm -f ./bin/debugMon
#	rm -f ../bin/debugMon
#	rm -f ../bin/eCmd
#	rm -f ../bin/elekIOServ
#	rm -f ../bin/elekStatus
#	rm -f ../bin/statView
#	rm -f ../bin/etalon
	rm -f ./spectrumAnalyzer/spectrumAnalyzer
	rm -f ./spectrumAnalyzer/*.o
	
#	$(MAKE) INSTDIR=$(INSTDIR) $(flags) -C instServer
#        rm ./instServer/*.o
#       rm ./instServer/instServer
#       rm ./bin/instServer
#       rm ../bin/instServer

	rm -f ./lib/*.o

