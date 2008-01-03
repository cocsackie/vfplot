# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.42 2007/12/22 00:37:43 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Itmp -I../include -DVERSION=\"$(VERSION)\" $(DFLAG)
LDFLAGS += -L. -l$(NAME)

LIBOBJ = arrow.o vfplot.o domain.o units.o vector.o bbox.o \
	 polyline.o aspect.o curvature.o hedgehog.o adaptive.o \
	 evaluate.o ellipse.o matrix.o cubic.o polynomial.o \
	 alist.o margin.o page.o dim0.o dim1.o dim2.o status.o \
	 slj.o contact.o bilinear.o mt.o rmdup.o dump.o \
	 gstack.o garray.o graph.o

LIBHDR = arrow.h vfplot.h error.h fill.h domain.h units.h \
	 vector.h bbox.h polyline.h aspect.h curvature.h \
	 hedgehog.h adaptive.h limits.h evaluate.h ellipse.h \
	 matrix.h cubic.h polynomial.h alist.h margin.h \
	 page.h dim0.h dim1.h dim2.h status.h nbs.h slj.h \
	 contact.h bilinear.h mt.h rmdup.h dump.h sincos.h \
	 gstack.h garray.h graph.h flag.h macros.h constants.h

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

SLJDOBJ  = slj-data.o
RUBBISH += $(SLJDOBJ) slj-data

slj-data.o : slj.c
	$(CC) slj.c $(CFLAGS) -DSLJ_DATA -c -o slj-data.o

slj-data : $(SLJDOBJ)
	$(CC) $(SLJDOBJ) $(LDLAGS) $(LDLIBS) -o slj-data

slj-data.eps : slj-data slj-data.sh
	sh slj-data.sh

slj-data-animate.gif : slj-data slj-data-animate.sh
	sh slj-data-animate.sh 

slj-data-minima.dat : slj-data slj-data-minima.sh
	sh slj-data-minima.sh > slj-data-minima.dat 

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



