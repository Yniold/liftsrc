#
# Makefile for ELectronic Konverter test tool 
#

CC=gcc
SHELL=/bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

cfgnear : cfgnear.c ../include/cfgopts.h
	$(CC) $(CFLAGS) -c cfgnear.c
	cp cfgnear.o ../lib
 
