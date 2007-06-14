# Makefile for cunit test-suite
# J.J.Green 2007
# $Id$

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o

RUBBISH += *~ $(OBJ) testsuite

testsuite : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o testsuite

clean : 
	$(RM) $(RUBBISH)

include Common.mk