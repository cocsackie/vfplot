# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.7 2002/11/17 16:57:03 jjg Exp jjg $

NAME     = vfplot

CFLAGS  += -g -Wall `gsl-config --cflags`
LDFLAGS += -lm `gsl-config --libs`

BGEN     = arwparse.output arwparse.vcg arwparse.c arwparse.h 
FGEN     = lex.backup arwscan.c 
GGEN     = options.c options.h
GEN      = $(BGEN) $(FGEN) $(OGEN)

COBJ     = stack.o
AOBJ     = arrow.o arwparse.o arwscan.o arwio.o arwtree.o
ATOBJ    = $(AOBJ) $(COBJ) arrow-test.o
OBJ      = $(AOBJ) $(COBJ) options.o main.o 

RUBBISH += *~ $(OBJ) $(ATOBJ) $(GEN) vfplot arrow-test arrow-test.dat \
	   $(NAME).aux $(NAME).dvi $(NAME).log $(NAME).ps

DPI      = 600

.PHONEY : all test clean install

all : view-arrow-test

view-arrow-test : arrow-test.dat arrow-test.gpt
	gnuplot -persist arrow-test.gpt

arrow-test.dat : arrow-test simple.arw
	./arrow-test < simple.arw > arrow-test.dat

clean :
	$(RM) $(RUBBISH)

show : $(NAME).ps
	gv $(NAME).ps

install :
	echo "steady now"

.PRECIOUS : $(NAME).c $(NAME).h $(NAME).tex $(NAME)-test.c 

$(NAME).ps : $(NAME).dvi
	dvips -Pcmz -Pamz -D$(DPI) $(NAME) -o

$(NAME).dvi : $(NAME).tex
	latex $(NAME)

arrow-test : $(ATOBJ)
	$(CC) $(ATOBJ) $(LDFLAGS) -o arrow-test 

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(NAME) 

$(FGEN) : arwscan.l arwparse.h
	flex -d -b -Cem -B -oarwscan.c arwscan.l

$(BGEN) : arwparse.y
	bison -t -v -g -d -o arwparse.c arwparse.y

$(GGEN) : options.ggo
	gengetopt -u -i options.ggo -f options -F options

main.o : $(GGEN)

# make the example vector field -- not many people have the
# software to do this

SPROW   = ../../../sprow/
SPECDIR = $(SPROW)/data/spec/
SPROWPM = $(SPROW)/src/perlmods
MASTER  = 20000762100_fed_wera
SLAVE   = 20000762110_lyn_wera

SITE    = fd

MSPEC   = $(SPECDIR)/$(SITE)/$(MASTER)
SSPEC   = $(SPECDIR)/$(SITE)/$(SLAVE)

RUBBISH += example.sea example.spr example.cpr 

example.sea :
	sea -v -s $(SITE) -t art:1:100 -o example.sea $(MSPEC) $(SSPEC)

example.spr : example.sea
	seaspr -v -o example.spr example.sea

example.cpr : example.spr
	sprcpr -o example.cpr example.spr

# this depends on example.cpr, but we dont want to recreate
# on every build, so it ommittted

example.vf :
	perl -I$(SPROWPM) cprvf.pl example.cpr > example.vf