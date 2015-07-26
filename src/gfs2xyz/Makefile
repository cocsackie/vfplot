# Makefile for the module arrow
# (c) J.J.Green 2015

NAME = gfs2xyz2D

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS +=
LDLIBS  +=

include ../Common.mk

GGEN = options.c options.h

OBJ = options.o main.o gfs2xyz.o

RUBBISH += *~ $(OBJ) $(NAME)

.PHONY : all test show clean spotless install reconfigure

all : $(NAME).1 $(NAME)

clean :
	$(RM) $(RUBBISH)

spotless : clean

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTBINEX) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

reconfigure :
	cd ../.. ; ./configure ; cd src/vfplot

# program

main.o : $(GGEN) main.c

options.o : CFLAGS += -Wno-unused-but-set-variable

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

ifdef GGO

$(GGEN) : options.ggo
	$(GGO) -u -N -i options.ggo -f options -F options
endif

# profiling, GPROF defined only if --enable-profile was used

ifdef GPROF

ZPLOG = profile-$(VERSION).log.gz

profile :
	echo "FIXME"

gmon.out :

$(ZPLOG) : gmon.out
	$(GPROF) $(NAME) | gzip > $(ZPLOG)

RUBBISH += gmon.out

endif
