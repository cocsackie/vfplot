# Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.20 2007/05/13 22:27:04 jjg Exp jjg $

PROGS = vfplot
DIRS  = $(PROGS) libvfplot testsuite

default :

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; make all || exit 1 ; done

install :
	for prog in $(PROGS) ; do cd $$prog ; make install || exit 1 ; done

libs :
	cd libvfplot ; $(MAKE) all install-local ; cd ..

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
