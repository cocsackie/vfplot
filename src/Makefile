# source Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.39 2009/01/06 19:53:52 jjg Exp jjg $

default : all

include Common.mk

LIBS   = libkdtree libvfplot 
BINEX  = vfplot
SRCEX  = sag2grd

ifdef WITH_GFS
BINEX += gfs2xyz
endif

PROGS = $(BINEX) $(SRCEX)

DIRS  = $(PROGS) $(LIBS) testsuite

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; $(MAKE) all || exit 1 ; cd .. ; done

libs :
	for lib in $(LIBS) ; do cd $$lib ; $(MAKE) all install-local || exit 1 ; cd .. ; done

install :
	for dir in $(PROGS) $(LIBS) ; do cd $$dir ; $(MAKE) install || exit 1 ; cd .. ; done

test check :
	cd testsuite ; $(MAKE) run-testsuite || exit 1 ; cd ..

ifdef GPROF

profile :
	for prog in $(PROGS) ; do cd $$prog ; $(MAKE) profile || exit 1 ; cd .. ; done

archive :
	cd vfplot ; $(MAKE) archive

else

profile archive :
	echo "no profiling support" || exit 1

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
