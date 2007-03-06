# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.5 2007/03/06 00:17:56 jjg Exp jjg $

NAME = vfplot

CFLAGS  += 
LDFLAGS += -L. -l$(NAME)

GGEN = options.c options.h
GEN  = $(GGEN)

OBJ    = options.o main.o plot.o circular.o
LIBOBJ = vfparrow.o vfplot.o

LIB = lib$(NAME).a

RUBBISH += *~ $(OBJ) $(LIBOBJ) $(NAME) $(LIB)

.PHONEY : all test clean install

all : $(NAME).1 $(NAME)

test : show

TYPE = circular

show : $(TYPE).ps
	gv $(TYPE).ps

clean :
	$(RM) $(RUBBISH)

spotless : clean
	$(RM) $(GEN)

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTEXEC) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

reconfigure :
	cd ../ ; ./configure ; cd src

# program

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME) 

$(GGEN) : options.ggo $(LIB)
	$(GGO) -u -i options.ggo -f options -F options

main.o : $(GGEN) main.c

# library

$(LIB) : $(LIBOBJ)
	ar rs $(LIB) $(LIBOBJ)

# test plots

RUBBISH += circular.ps

A4PAPER  = -g 210x297m
A5PAPER  = -g 148x210m
A6PAPER  = -g 105x148m

circular.ps : $(NAME) Makefile
	./$(NAME) -v -t circular $(A6PAPER) -n 150 -o circular.ps

include Common.mk
