# Makefile.in for vfplot

RUBBISH   = *~ 
CONFFILES = config.cache config.log config.status configure

# targets

default : all

include src/Common.mk

all : 
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

dist : man txt veryclean 
	cd .. ; \
	cp -r vfplot vfplot-$(VERSION) ;\
	cd vfplot-$(VERSION) ;\
	$(MAKE) spotless ;\
	cd .. ; \
	tar --exclude=RCS -zcvf snapshot/vfplot-$(VERSION).tar.gz \
	  vfplot-$(VERSION)

maint-dist : clean
	cd .. ; \
	tar -zcvf vfplot-maint.tar.gz vfplot ; \
	cd vfplot

