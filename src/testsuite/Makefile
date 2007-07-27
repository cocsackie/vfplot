# Makefile for cunit test-suite
# J.J.Green 2007
# $Id: Makefile,v 1.7 2007/07/10 22:44:01 jjg Exp jjg $

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o cubic.o ellipse.o bbox.o vector.o arrow.o \
	margin.o lennard.o contact.o

RUBBISH += *~ $(OBJ) testsuite

default : all

all : testsuite

run-testsuite : testsuite
	./testsuite

testsuite : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o testsuite

clean spotless veryclean : 
	$(RM) $(RUBBISH)

include ../Common.mk