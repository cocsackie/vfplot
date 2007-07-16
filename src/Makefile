# source Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.21 2007/06/14 21:29:36 jjg Exp jjg $

PROGS = vfplot
LIBS  = libvfplot lib$(DELAUNAY)
DIRS  = $(PROGS) $(LIBS) testsuite

default :

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; make all || exit 1 ; done

install :
	for prog in $(PROGS) ; do cd $$prog ; make install || exit 1 ; done

libs :
	for lib in $(LIBS) ; do cd $$lib ; make all install-local || exit 1 ; done

test check :
	cd testsuite ; $(MAKE) run-testsuite || exit1 ; cd ..

clean :
	for dir in $(DIRS) ; do \
	  cd $$dir ; \
	  make clean || exit 1 ; \
	  cd .. ; \
	done

spotless veryclean :
	for dir in $(DIRS) ; do \
	  cd $$dir ; \
	  make spotless || exit 1 ; \
	  cd .. ; \
	done
	$(RM) Common.mk include/config.h

.PHONEY : spotless veryclean clean test check libs install all

include Common.mk