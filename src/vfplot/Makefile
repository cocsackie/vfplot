# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.32 2007/10/14 22:01:08 jjg Exp jjg $

NAME = vfplot

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib 
LDLIBS  += -lvfplot -lkdtree 

GGEN = options.c options.h

OBJ = options.o main.o plot.o circular.o electro.o cylinder.o field.o

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

SIZE = -w4i

eps : $(EPS)

circular.ps : $(NAME) Makefile
	./$(NAME) -d circular.dom -v -s 10 -E -D1 -P0.5 \
	          -p adaptive -t circular $(SIZE) -m 4/4/0 -o circular.ps

electro2.ps : $(NAME) Makefile
	./$(NAME) -v -f255 -D2 -s0.25 -E -t electro2 $(SIZE) \
	          -p adaptive -m 4/4/0 -o electro2.ps

electro3.ps : $(NAME) Makefile
	./$(NAME) -v -f255 -S shortest -D2 -s0.25 -t electro3 $(SIZE) \
	          -p adaptive -m 4/4/0 -o electro3.ps

cylinder.ps cylinder.dat : $(NAME) Makefile
	./$(NAME) -v -f150 -D2 -s1 -t cylinder $(SIZE) \
		--dump-vectors cylinder.dat -i5/50 -p adaptive -P0 -E -o cylinder.ps 

# vector dump and plot - you need GMT for this

RUBBISH += cylinder.u.grd cylinder.v.grd cylinder.dat cylinder-grd2.ps
CYLGRD = GMT xyz2grd -I0.015625/0.015625 -R-1/1/-1/1 -F

cylinder.u.grd cylinder.v.grd : cylinder.dat
	cut -f 1,2,3 cylinder.dat | $(CYLGRD) -Gcylinder.u.grd
	cut -f 1,2,4 cylinder.dat | $(CYLGRD) -Gcylinder.v.grd

cylinder-grd2.ps : cylinder.u.grd cylinder.v.grd
	./$(NAME) -v -f150 -D2 -s1 -F grd2 -i5/50 -p adaptive -P0 -E -o cylinder-grd2.ps \
		$(SIZE) cylinder.u.grd cylinder.v.grd

# animation - you need imagemagick and mencoder for this

cylinder.avi : $(NAME) Makefile
	./$(NAME) -v -D2 -s0.9 -t cylinder $(SIZE) \
	          --animate -i150/10 -P0 -p adaptive -E -o cylinder.ps 
	ls -1 anim.*.*.eps | xargs -P2 -n20 mogrify -format png 
	rm anim.*.*.eps
	mencoder 'mf://anim.*.png' -mf w=800:h=600:fps=20:type=png \
	  -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o cylinder.avi
	rm anim.*.*.png
