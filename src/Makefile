# Makefile
# (c) J.J.Green 2007
# $Id: Makefile,v 1.17 2007/05/11 23:49:48 jjg Exp jjg $

PROGS = vfplot

default :

all : libs 
	for prog in $(PROGS) ; do cd $$prog ; make all && exit 1 ; done

install :
	for prog in $(PROGS) ; do cd $$prog ; make install && exit 1 ; done

libs :
	cd libvfplot ; $(MAKE) all install-local ; cd ..

include Common.mk
