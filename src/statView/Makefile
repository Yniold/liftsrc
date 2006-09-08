#
# Makefile for ELectronic Konverter test tool 
#

VERSION = 0.1
PROJECT=statView

CC=gcc
SHELL=/bin/sh

INSTDIR?=../bin

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC
LDFLAGS :=-lrt
endif

CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb


modules=

headers=

.SUFFIXES:
.SUFFIXES: .c .o
 
$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC)  $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

