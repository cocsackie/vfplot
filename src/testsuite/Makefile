# Makefile for cunit test-suite
# J.J.Green 2007
# $Id: Makefile,v 1.10 2007/12/12 23:07:41 jjg Exp jjg $

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

ifdef WITH_UNIT

testsuite : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o testsuite

else

testsuite : 
	echo "no CUnit support, use --enable-tests"
	exit 1

endif


clean spotless veryclean : 
	$(RM) $(RUBBISH)

include ../Common.mk