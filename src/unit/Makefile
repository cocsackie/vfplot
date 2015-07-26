# Makefile for cunit test-suite
# J.J.Green 2015

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o polynomial.o cubic.o ellipse.o bbox.o vector.o arrow.o \
	margin.o potential.o contact.o bilinear.o sagread.o

RUBBISH += *~ $(OBJ) unit

.PHONY : run all default clean spotless veryclean

default : all

include ../Common.mk

all : unit

run : unit
	./unit

ifdef WITH_UNIT

unit : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o unit

else

.PHONY : unit

unit :
	echo "no CUnit support, use --enable-tests"
	exit 1

endif

clean spotless veryclean :
	$(RM) $(RUBBISH)
