# Makefile for cunit test-suite
# J.J.Green 2015

LDLIBS  += -lvfplot
LDFLAGS += -L../lib/
CFLAGS  += -I../include/

OBJ = main.o tests.o fixture.o \
	assert_bbox.o \
	assert_matrix.o \
	assert_polyline.o \
	assert_vector.o \
	test_arrow.o \
	test_aspect.o \
	test_bbox.o \
	test_bilinear.o \
	test_contact.o \
	test_cubic.o \
	test_curvature.o \
	test_domain.o \
	test_ellipse.o \
	test_margin.o \
	test_matrix.o \
	test_polyline.o \
	test_polynomial.o \
	test_potential.o \
	test_sagread.o \
	test_units.o \
	test_vector.o

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
