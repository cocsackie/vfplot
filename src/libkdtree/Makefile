# vfplot Makefile for the kdtree library
# (c) J.J.Green 2007
# $Id: Makefile,v 1.5 2007/11/27 22:31:08 jjg Exp jjg $

NAME = kdtree

CFLAGS  += -I../include
LDFLAGS += 

LIBOBJ = kdtree.o
LIBHDR = kdtree.h
LIB    = lib$(NAME).a

RUBBISH += *~ $(LIBOBJ) $(LIB) 

.PHONEY : all test clean install

all : $(LIB)

clean :
	$(RM) $(RUBBISH)

spotless : clean uninstall-local

install :

install-local : all
	$(INSTDATA) $(LIB) ../lib/$(LIB)
	ranlib ../lib/$(LIB)
	for h in $(LIBHDR) ; do \
	 $(INSTDATA) $$h ../include/$$h ;\
	done

uninstall-local :
	$(RM) ../lib/$(LIB)
	for h in $(LIBHDR) ; do $(RM) ../include/$$h ; done

$(LIB) : $(LIBOBJ)
	ar rs $(LIB) $(LIBOBJ)

include ../Common.mk