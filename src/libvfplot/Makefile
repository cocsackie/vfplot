# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.3 2007/05/28 20:28:34 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Itmp -DVERSION=\"$(VERSION)\"
LDFLAGS += -L. -l$(NAME)

LIBOBJ = arrow.o vfplot.o domain.o units.o vector.o bbox.o \
	 polyline.o aspect.o curvature.o hedgehog.o adaptive.o \
	 evaluate.o

LIBHDR = arrow.h vfplot.h error.h fill.h domain.h units.h \
	 vector.h bbox.h polyline.h aspect.h curvature.h \
	 hedgehog.h adaptive.h limits.h evaluate.h

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

# library

$(LIB) : $(LIBOBJ)
	ar rs $(LIB) $(LIBOBJ)

tmp/vfplot :
	cd tmp ; ln -s .. vfplot ; cd ..

include ../Common.mk

