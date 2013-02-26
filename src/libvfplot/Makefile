# Makefile for the module arrow
# (c) J.J.Green 2008
# $Id: Makefile,v 1.60 2009/01/05 00:03:26 jjg Exp $

NAME = vfplot

CFLAGS  += -Itmp -I../include -DVERSION=\"$(VERSION)\" $(DFLAG)
LDFLAGS += -L. -l$(NAME)

LIBOBJ = arrow.o vfplot.o domain.o units.o vector.o bbox.o \
	 polyline.o aspect.o curvature.o hedgehog.o adaptive.o \
	 evaluate.o ellipse.o matrix.o cubic.o polynomial.o \
	 margin.o page.o dim0.o dim1.o dim2.o status.o \
	 contact.o bilinear.o mt.o rmdup.o sagwrite.o sincos.o \
	 sagread.o gstack.o garray.o graph.o paths.o potential.o \
	 gstate.o

LIBHDR = arrow.h vfplot.h error.h fill.h domain.h units.h \
	 vector.h bbox.h polyline.h aspect.h curvature.h \
	 hedgehog.h adaptive.h limits.h evaluate.h ellipse.h \
	 matrix.h cubic.h polynomial.h paths.h margin.h \
	 page.h dim0.h dim1.h dim2.h status.h nbs.h contact.h \
	 bilinear.h mt.h rmdup.h sagwrite.h sagread.h \
	 sincos.h gstack.h garray.h graph.h flag.h macros.h \
	 constants.h potential.h gstate.h

LIB = lib$(NAME).a

RUBBISH += *~ $(LIBOBJ) $(LIB) 

.PHONEY : all test clean install

all : tmp/vfplot $(LIB)

clean :
	$(RM) $(RUBBISH)

spotless : clean uninstall-local
	$(RM) tmp/vfplot

install :
	$(INSTDIR) $(MAN5)
	for file in sag.5 dom.5 vgs.5 vfplot-povray.5 ; do \
	  $(INSTDATA) $$file $(MAN5)/$$file ;\
	done

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



