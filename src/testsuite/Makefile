# Makefile for cunit test-suite
# J.J.Green 2007
# $Id: Makefile,v 1.9 2007/08/16 22:43:44 jjg Exp jjg $

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o cubic.o ellipse.o bbox.o vector.o arrow.o \
	margin.o slj.o contact.o bilinear.o

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