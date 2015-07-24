# Makefile for cunit test-suite
# J.J.Green 2007

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o cubic.o ellipse.o bbox.o vector.o arrow.o \
	margin.o potential.o contact.o bilinear.o

RUBBISH += *~ $(OBJ) testsuite

default : all

include ../Common.mk

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
