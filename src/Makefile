# Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.18 2007/05/13 21:31:17 jjg Exp jjg $

PROGS = vfplot

default :

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; make all || exit 1 ; done

install :
	for prog in $(PROGS) ; do cd $$prog ; make install || exit 1 ; done

libs :
	cd libvfplot ; $(MAKE) all install-local ; cd ..

clean :
	for dir in libvfplot $(PROGS) ; do \
	  cd $$dir ; \
	  make clean || exit 1 ; \
	  cd .. ; \
	done

spotless veryclesn :
	for dir in libvfplot $(PROGS) ; do \
	  cd $$dir ; \
	  make spotless || exit 1 ; \
	  cd .. ; \
	done
	$(RM) Common.mk include/config.h

include Common.mk
