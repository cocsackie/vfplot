# source Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.34 2008/01/20 20:22:58 jjg Exp jjg $

default : all

include Common.mk

LIBS   = libkdtree libvfplot 
PROGS  = vfplot

ifdef WITH_GFS
PROGS += gfs2xyz
endif

DIRS   = $(PROGS) $(LIBS) testsuite

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; $(MAKE) all || exit 1 ; cd .. ; done

install :
	for prog in $(PROGS) ; do cd $$prog ; $(MAKE) install || exit 1 ; cd .. ; done

libs :
	for lib in $(LIBS) ; do cd $$lib ; $(MAKE) all install-local || exit 1 ; cd .. ; done

test check :
	cd testsuite ; $(MAKE) run-testsuite || exit 1 ; cd ..

profile :
ifdef GPROF
	for prog in $(PROGS) ; do cd $$prog ; $(MAKE) profile || exit 1 ; cd .. ; done
else
	echo "no profiling support"
endif

clean :
	for dir in $(DIRS) ; do \
	  cd $$dir ; \
	  $(MAKE) clean || exit 1 ; \
	  cd .. ; \
	done

spotless veryclean :
	for dir in $(DIRS) ; do \
	  cd $$dir ; \
	  $(MAKE) spotless || exit 1 ; \
	  cd .. ; \
	done
	$(RM) Common.mk include/config.h

.PHONEY : spotless veryclean clean test check libs install all
