#
# Makefile for Spectrum Analyzer Software
#

VERSION=0.1
PROJECT=spectrumAnalyzer

SHELL=/bin/sh

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
LDFLAGS+=-lusb
LDFLAGS+=-ldirectfb
LDFLAGS+=-lpthread
LDFLAGS+=-I/usr/local/include 
LDFLAGS+=-I/usr/include/directfb
LDFLAGS+=-L.
LDFLAGS+=-lnsl
LDFLAGS+=-lm
LDFLAGS+=-lc
LDFLAGS+=-L/usr/local/lib
INSTDIR+=../bin/

# (root$)gcc -o usbdevice_details testlibusb.c -I/usr/local/include -L. -lnsl -lm -lc -L/usr/local/lib 
# -lusb 

modules=

headers=

.SUFFIXES:
.SUFFIXES: .c .o

$(PROJECT) : $(PROJECT).c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECT).c $(modules) -o $(PROJECT) $(LDFLAGS)
	     cp $(PROJECT) $(INSTDIR)

clean: 
	rm -f *.o
	rm -f spectrumAnalyzer
	
all: $(PROJECT)