# vfplot Makefile for the triangle library
# (c) J.J.Green 2007
# $Id: Makefile,v 1.2 2007/09/16 16:35:08 jjg Exp jjg $

NAME = kdtree

# REDUCED    - omit research code  
# CDT_ONLY   - include only constrained triangulation
# TRILIBRARY - omit command-line interface

CFLAGS  += 
LDFLAGS += -L../include

LIBOBJ = kdtree.o
LIBHDR = kdtree.h
LIB    = lib$(NAME).a

RUBBISH += *~ $(LIBOBJ) $(LIB) 

.PHONEY : all test clean install

all : $(LIB)

clean :
	$(RM) $(RUBBISH)

spotless : clean uninstall-local

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