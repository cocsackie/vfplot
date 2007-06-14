# Makefile for cunit test-suite
# J.J.Green 2007
# $Id: Makefile,v 1.1 2007/06/14 20:40:41 jjg Exp jjg $

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o

RUBBISH += *~ $(OBJ) testsuite

testsuite : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o testsuite

clean : 
	$(RM) $(RUBBISH)

include ../Common.mk