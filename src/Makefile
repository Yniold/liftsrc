#
# Makefile for ELectronic Konverter test tool 
#

VERSION=08
PROJECT=all


CC:=gcc
SHELL:=/bin/sh
INSTDIR:=$(shell /bin/pwd)/bin/
LIBDIR:=$(shell /bin/pwd)/lib/


flags= -j -k 

CFLAGS=-fpack-struct

#modules=lib/elekIO.o lib/udptools.o
#headers=include/elekIO.h include/elekIOPorts.h

modules=
headers=

.SUFFIXES:
.SUFFIXES: .c .o

all : $(modules) $(headers)

	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C cfgnear 
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C commTools 
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(LIBDIR) $(flags) -C elekIO

	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C eCmd
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekIOServ
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C elekStatus
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C statView
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C etalon
	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C debugMon
#	$(MAKE) CFLAGS=$(CFLAGS) INSTDIR=$(INSTDIR) $(flags) -C instServer
	chmod +x $(INSTDIR)* 
	cp $(INSTDIR)* ../bin
#	cp ../bin/* /tmp/lift
#	tar -C ../ -czf /mnt/magera/HorusV$(VERSION).tgz src

clean:

	rm -f ./cfgnear/*.o
	rm -f ./commTools/*.o
	rm -f ./elekIO/*.o
	rm -f ./eCmd/eCmd
	rm -f ./bin/eCmd
	rm -f ../bin/eCmd
	rm -f ./elekIOServ/elekIOServ
	rm -f ./bin/elekIOServ
	rm -f ../bin/elekIOServ
	rm -f ./elekStatus/elekStatus
	rm -f ./bin/elekStatus
	rm -f ../bin/elekStatus
	rm -f ./statView/statView
	rm -f ./bin/statView
	rm -f ../bin/statView
	rm -f ./etalon/etalon
	rm -f ./bin/etalon
	rm -f ../bin/etalon
	rm -f ./debugMon/debugMon
	rm -f ./bin/debugMon
	rm -f ../bin/debugMon
#	$(MAKE) INSTDIR=$(INSTDIR) $(flags) -C instServer
#        rm ./instServer/*.o
#       rm ./instServer/instServer
#       rm ./bin/instServer
#       rm ../bin/instServer

	rm -f ./lib/*.o

