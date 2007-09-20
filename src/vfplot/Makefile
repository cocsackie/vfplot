# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.24 2007/09/20 21:23:32 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib 
LDLIBS  += -l$(NAME) -l$(NBODY)

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
	          -i1/50 --network-pen 0.2 -p adaptive -P0 -E -o cylinder.ps 

RUBBISH +=

cylinder.avi : $(NAME) Makefile
	./$(NAME) -v -D2 -s0.9 -t cylinder $(SIZE) \
	          --animate -i100/10 -P0 -p adaptive -E -o cylinder.ps 
	mogrify -format png anim.*.*.eps
	rm anim.*.*.eps
	mencoder 'mf://anim.*.png' -mf w=800:h=600:fps=20:type=png \
	  -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o cylinder.avi
	rm anim.*.*.png
