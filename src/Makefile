# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile.in,v 1.3 2002/11/20 00:11:17 jjg Exp jjg $

NAME     = vfplot

CFLAGS  += -g -O3 -Wall
LDFLAGS += -lm 
# LDFLAGS += -lefence 

GGEN     = options.c options.h
GEN      = $(GGEN)

OBJ      = arrow.o options.o main.o vfplot.o circular.o

RUBBISH += *~ $(OBJ) vfplot

.PHONEY : all test clean install

all : test

test : show

TYPE = circular

show : $(TYPE).ps
	gv $(TYPE).ps

clean :
	$(RM) $(RUBBISH)

spotless : clean
	$(RM) $(GEN)

install :
	echo "steady now"

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(NAME) 

$(GGEN) : options.ggo
	gengetopt -u -i options.ggo -f options -F options

main.o : $(GGEN)

# test plots

RUBBISH += circular.ps

circular.ps : vfplot Makefile
	./vfplot -v -t circular -g 596x842p -n 500 -o circular.ps

include Common.mk
