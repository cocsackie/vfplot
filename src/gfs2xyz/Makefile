# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.2 2007/11/15 00:27:49 jjg Exp jjg $

NAME = gfs2xyz

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib 
LDLIBS  += -lvfplot

include ../Common.mk

GGEN = options.c options.h

OBJ = options.o main.o gfs2xyz.o

RUBBISH += *~ $(OBJ) $(NAME)

.PHONEY : all test show clean spotless install reconfigure

all : $(NAME).1 $(NAME)

clean :
	$(RM) $(RUBBISH)

spotless : clean

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTEXEC) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

reconfigure :
	cd ../.. ; ./configure ; cd src/vfplot

# program

main.o : $(GGEN) main.c

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

ifdef GGO

$(GGEN) : options.ggo
	$(GGO) -u -N -i options.ggo -f options -F options
endif

# profiling, GPROF defined only if --enable-profile was used

ifdef GPROF

gmon.out : test

profile.log : gmon.out
	$(GPROF) $(NAME) > profile.log

RUBBISH += gmon.out profile.log

endif

# if you have configured with --enable-dmalloc the this has debugging
# info on memory usage, leaks etc

DMO = log-known,log-non-free,log-stats,error-free-null,log-trans
RUBBISH += dmalloc.log

dmalloc.log : 
	export DMALLOC_OPTIONS=$(DMO),log=dmalloc.log ; $(MAKE) test
