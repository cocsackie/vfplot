# source Makefile
# (c) J.J.Green 2007

default : all

include Common.mk

LIBS  = libkdtree libvfplot
BINEX = vfplot
SRCEX = sag2grd

ifdef WITH_GFS
BINEX += gfs2xyz
endif

PROGS = $(BINEX) $(SRCEX)
DIRS  = $(PROGS) $(LIBS) unit

all : libs
	for prog in $(PROGS) ; do $(MAKE) -C $$prog all ; done

libs :
	for lib in $(LIBS) ; do $(MAKE) -C $$lib all install-local ; done

install :
	for dir in $(PROGS) $(LIBS) ; do $(MAKE) -C $$dir install ; done

test check : unit accept

unit :
	cd unit ; $(MAKE) run || exit 1 ; cd ..

accept :
	for prog in $(PROGS) ; do $(MAKE) -C $$prog accept || exit 1 ; done

ifdef GPROF

profile :
	$(MAKE) -C vfplot profile

archive :
	$(MAKE) -C vfplot archive

else

profile archive :
	echo "no profiling support"
	exit 1

endif

clean :
	for dir in $(DIRS) ; do $(MAKE) -C $$dir clean ; done

spotless veryclean :
	for dir in $(DIRS) ; do $(MAKE) -C $$dir spotless ; done
	$(RM) Common.mk include/config.h

.PHONY : spotless veryclean clean test check libs install all unit accept
