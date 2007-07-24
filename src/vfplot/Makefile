# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.16 2007/07/23 23:52:14 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib 
LDLIBS  += -l$(NAME) -l$(DELAUNAY)

GGEN = options.c options.h

OBJ = options.o main.o plot.o circular.o electro.o cylinder.o

RUBBISH += *~ $(OBJ) $(NAME)

.PHONEY : all test show clean spotless install reconfigure

all : $(NAME).1 $(NAME)

test : show

TYPE = cylinder

show : $(TYPE).ps
	gv $(TYPE).ps

clean :
	$(RM) $(RUBBISH)

spotless : clean
	$(RM) $(GGEN)

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTEXEC) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

reconfigure :
	cd ../.. ; ./configure ; cd src/vfplot

include ../Common.mk

# program

main.o : $(GGEN) main.c

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

ifdef GGO
$(GGEN) : options.ggo
	$(GGO) -u -N -i options.ggo -f options -F options
endif

# test plots

EPS = circular.ps electro2.ps electro3.ps cylinder.ps 
RUBBISH += $(EPS)

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
	./$(NAME) -v -f150 -D2 -s1 -S bendiest -t cylinder $(SIZE) \
	          -N0.2 -p adaptive -P0.5 -o cylinder.ps 


