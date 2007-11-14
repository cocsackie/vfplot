# source Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.31 2007/11/06 15:38:49 jjg Exp jjg $

default : all

include Common.mk

LIBS   = libkdtree libvfplot 
PROGS  = vfplot
ifeq ($(WITH_GFS),yes)
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
