# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.12 2007/07/02 21:29:40 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -Wall -g -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib -l$(NAME)
LDLIBS  += 

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
	$(GGO) -u -N -i options.ggo -f options -F options

main.o : $(GGEN) main.c

# test plots

EPS = circular.ps electro2.ps electro3.ps cylinder.ps 

RUBBISH += $(EPS)

A4PAPER  = -g 210x297m
A5PAPER  = -g 148x210m
A6PAPER  = -g 105x148m

SIZE = -w 4i

eps : $(EPS)

circular.ps : $(NAME) Makefile
	./$(NAME) -d circular.dom -v -s 10 -E -D1 -P0.5 \
	          -p adaptive -t circular $(SIZE) -n 150 -o circular.ps

electro2.ps : $(NAME) Makefile
	./$(NAME) -v -f255 -D2 -s1 -E -t electro2 $(SIZE) \
	          -p adaptive -n 150 -o electro2.ps

electro3.ps : $(NAME) Makefile
	./$(NAME) -v -f255 -S shortest -D2 -s1 -t electro3 $(SIZE) \
	          -p adaptive -n 250 -o electro3.ps

cylinder.ps : $(NAME) Makefile
	./$(NAME) -v -f255 -D2 -s1 -S bendiest -t cylinder $(SIZE) \
	          -E -p adaptive -n 200 -o cylinder.ps 

include ../Common.mk
