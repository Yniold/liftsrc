#
# Makefile for ELectronic Konverter test tool 
#

VERSION = 0.1
PROJECT = instServer

CC=gcc
SHELL=/bin/sh

modules=../lib/cfgnear.o logerror.o main.o

headers=../include/cfgopts.h protos.h

.SUFFIXES:
.SUFFIXES: .c .o
 
instServer : main.c $(modules) $(headers)
	     echo $(PROJECT) $(VERSION)
	     $(CC) $(modules) -o instServer -lrt -ggdb
	     cp instServer /tmp/lift

