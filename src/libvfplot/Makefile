# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.9 2007/06/12 22:53:20 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Itmp -DVERSION=\"$(VERSION)\"
LDFLAGS += -L. -l$(NAME)

LIBOBJ = arrow.o vfplot.o domain.o units.o vector.o bbox.o \
	 polyline.o aspect.o curvature.o hedgehog.o adaptive.o \
	 evaluate.o ellipse.o matrix.o cubic.o polynomial.o \
	 alist.o

LIBHDR = arrow.h vfplot.h error.h fill.h domain.h units.h \
	 vector.h bbox.h polyline.h aspect.h curvature.h \
	 hedgehog.h adaptive.h limits.h evaluate.h ellipse.h \
	 matrix.h cubic.h polynomial.h alist.h

LIB = lib$(NAME).a

RUBBISH += *~ $(LIBOBJ) $(LIB) 

.PHONEY : all test clean install

all : tmp/vfplot $(LIB)

clean :
	$(RM) $(RUBBISH)

spotless : clean uninstall-local
	$(RM) tmp/vfplot

install-local : all
	$(INSTDATA) $(LIB) ../lib/$(LIB)
	ranlib ../lib/$(LIB)
	for h in $(LIBHDR) ; do \
	  $(INSTDATA) $$h ../include/vfplot/$$h ;\
	done

uninstall-local :
	$(RM) ../lib/$(LIB)
	for h in $(LIBHDR) ; do $(RM) ../include/vfplot/$$h ; done

# program

DTOBJ    = domain-test.o
RUBBISH += $(DTOBJ) domain-test

domain-test : $(DTOBJ) $(LIB)
	$(CC) $(DTOBJ) $(LDFLAGS) $(LDLIBS) -o domain-test


ETOBJ    = ellipse-test.o cubic.o polynomial.o 
RUBBISH += $(ETOBJ) ellipse-test

ellipse-test.o : ellipse.c
	$(CC) -c -o ellipse-test.o $(CFLAGS) -DETP_MAIN ellipse.c

ellipse-test : ellipse-test.o $(ETOBJ)
	$(CC) $(ETOBJ) $(LDFLAGS) $(LDLIBS) -o ellipse-test

PTOBJ = polynomial-test.o
RUBBISH += $(PTOBJ) polynomail-test

polynomial-test.o : polynomial.c
	$(CC) -c -o polynomial-test.o $(CFLAGS) -DPT_MAIN polynomial.c

polynomial-test : polynomial-test.o $(PTOBJ)
	$(CC) $(PTOBJ) $(LDFLAGS) $(LDLIBS) -o polynomial-test

# library

$(LIB) : $(LIBOBJ)
	ar rs $(LIB) $(LIBOBJ)

tmp/vfplot :
	cd tmp ; ln -s .. vfplot ; cd ..

include ../Common.mk

