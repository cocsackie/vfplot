# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.26 2007/08/10 23:38:56 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Itmp -I../include -DVERSION=\"$(VERSION)\" $(DFLAG)
LDFLAGS += -L. -l$(NAME) -l$(DELAUNAY)

LIBOBJ = arrow.o vfplot.o domain.o units.o vector.o bbox.o \
	 polyline.o aspect.o curvature.o hedgehog.o adaptive.o \
	 evaluate.o ellipse.o matrix.o cubic.o polynomial.o \
	 alist.o margin.o page.o dim0.o dim1.o dim2.o status.o \
	 lennard.o contact.o bilinear.o

LIBHDR = arrow.h vfplot.h error.h fill.h domain.h units.h \
	 vector.h bbox.h polyline.h aspect.h curvature.h \
	 hedgehog.h adaptive.h limits.h evaluate.h ellipse.h \
	 matrix.h cubic.h polynomial.h alist.h margin.h \
	 page.h dim0.h dim1.h dim2.h status.h nbs.h lennard.h \
	 contact.h bilinear.h

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

LJDOBJ   = lennard-data.o polynomial.o
RUBBISH += $(LJDOBJ) lennard-data

lennard-data.o : lennard.c
	$(CC) lennard.c $(CFLAGS) -DPLOTDATA -c -o lennard-data.o

lennard-data : $(LJDOBJ)
	$(CC) $(LJDOBJ) $(LDLAGS) $(LDLIBS) -o lennard-data

LJBOBJ   = lennard-bvals.o cubic.o polynomial.o
RUBBISH += $(LJBOBJ) lennard-bvals

lennard-bvals.o : lennard.c
	$(CC) lennard.c $(CFLAGS) -DBVALS -c -o lennard-bvals.o

lennard-bvals : $(LJBOBJ)
	$(CC) $(LJBOBJ) $(LDLAGS) $(LDLIBS) -o lennard-bvals

CTCOBJ   = contact-fvals.o matrix.o vector.o ellipse.o cubic.o polynomial.o
RUBBISH += $(CTCOBJ) contact-fvals

contact-fvals.o : contact.c
	$(CC) contact.c $(CFLAGS) -DFVALS -c -o contact-fvals.o

contact-fvals : $(CTCOBJ)
	$(CC) $(CTCOBJ) $(LDLAGS) $(LDLIBS) -o contact-fvals

BLTOBJ   = bilinear-test.o bbox.o
RUBBISH += $(BLTOBJ) bilinear-test

bilinear-test.o : bilinear.c
	$(CC) bilinear.c $(CFLAGS) -DTEST -c -o bilinear-test.o

bilinear-test : $(BLTOBJ)
	$(CC) $(BLTOBJ) $(LDLAGS) $(LDLIBS) -o bilinear-test

# library

$(LIB) : $(LIBOBJ)
	ar rs $(LIB) $(LIBOBJ)

tmp/vfplot :
	cd tmp ; ln -s .. vfplot ; cd ..

include ../Common.mk

# must be after common include

ifeq ("$(NBODY)","triangle")
DFLAG = -DTRIANGLE
else 
ifeq ("$(NBODY)","dpmta")
DFLAG = -DDPMTA
endif
endif
