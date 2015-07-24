# Makefile.in for vfplot

RUBBISH   = *~
CONFFILES = config.cache config.log config.status configure

# targets

default : all

vpath %.xml docbook
include src/Common.mk

all : txt
	$(MAKE) -C src all

install :
	$(MAKE) -C src install

test check :
	$(MAKE) -C src test

profile :
	$(MAKE) -C src profile

archive :
	$(MAKE) -C src archive

txt : CREDITS.txt CHANGES.txt INSTALL.txt

clean :
	$(MAKE) -C src clean
	$(RM) $(RUBBISH)

spotless veryclean :
	$(MAKE) -C src veryclean
	$(RM) $(CONFFILES) $(RUBBISH)
	autoconf
	rm -rf autom4te.cache/

DIST = vfplot-$(VERSION)
TAROPT = --exclude-from=$(DIST)/.distignore

dist : all veryclean
	cd .. ; \
	cp -pr vfplot $(DIST) ; \
	tar $(TAROPT) -zpcvf snapshot/$(DIST).tar.gz $(DIST)

maint-dist : clean
	cd .. ; \
	tar -zcvf vfplot-maint.tar.gz vfplot ; \
	cd vfplot
