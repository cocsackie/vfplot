# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.17 2007/05/11 23:49:48 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Wall -g -I../include
LDFLAGS += -L../lib -l$(NAME)

GGEN = options.c options.h
GEN  = $(GGEN)

OBJ = options.o main.o plot.o circular.o electro.o cylinder.o

RUBBISH += *~ $(OBJ) $(NAME)

.PHONEY : all test clean install

all : $(NAME).1 $(NAME)

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

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
	cd ../.. ; ./configure ; cd src/vfplot

# program

$(GGEN) : options.ggo $(LIB)
	$(GGO) -u -i options.ggo -f options -F options

main.o : $(GGEN) main.c

# test plots

RUBBISH += circular.ps electro2.ps electro3.ps cylinder.ps

A4PAPER  = -g 210x297m
A5PAPER  = -g 148x210m
A6PAPER  = -g 105x148m

circular.ps : $(NAME) Makefile
	./$(NAME) -d ../libvfplot/simple.dom -v -f 255 -s 2 -E \
	  -t circular $(A4PAPER) -n 150 -o circular.ps

electro2.ps : $(NAME) Makefile
	./$(NAME) -v -f 255 -s 2 -E -t electro2 $(A6PAPER) -n 150 -o electro2.ps

electro3.ps : $(NAME) Makefile
	./$(NAME) -v -f 255 -s 2 -t electro3 $(A6PAPER) -n 250 -o electro3.ps

cylinder.ps : $(NAME) Makefile
	./$(NAME) -v -s1 -t cylinder $(A6PAPER) -n 200 -o cylinder.ps 

include ../Common.mk