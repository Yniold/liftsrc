#
# Makefile for ELectronic Konverter test tool 
#

VERSION = 0.1
PROJECT=statView

CC=gcc
SHELL=/bin/sh

modules=

headers=

.SUFFIXES:
.SUFFIXES: .c .o
 
$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC)  $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) -lrt -ggdb
	     cp $(PROJECT) $(INSTDIR)

