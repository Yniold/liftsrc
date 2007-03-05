#
# Makefile for Spectrum Analyzer Software
#

VERSION=0.1
PROJECT=spectrometerServer

SHELL=/bin/sh

ifeq ($(ARMBUILD),1)
CPPFLAGS :=-DRUNONARM
CROSS_COMPILE=arm-linux-
else
CROSS_COMPILE=
CPPFLAGS :=-DRUNONPC -static
LDFLAGS :=-lrt
endif

ifneq ($(ARMBUILD),1)
CC := $(CROSS_COMPILE)gcc
export CC

LDFLAGS+=-ggdb
LDFLAGS+=-lusb
LDFLAGS+=-lm
LDFLAGS+=-L/usr/local/lib
INSTDIR+=../bin/


modules=

headers=

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

all: $(PROJECT)

endif

clean: 
	rm -f *.o
	rm -f spectrometerServer
	
