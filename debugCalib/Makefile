#
# Makefile for ELectronic Konverter test tool
#

VERSION = 0.1
PROJECT=debugCalib

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

modules=
headers=

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC)  $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

