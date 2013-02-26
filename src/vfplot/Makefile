# Makefile for the module arrow
# (c) J.J.Green 2002
# $Id: Makefile,v 1.80 2012/05/22 23:50:00 jjg Exp $

NAME = vfplot

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib 
LDLIBS  += -lvfplot -lkdtree

ADMIN = ../../..

include ../Common.mk

GGEN = options.c options.h

OBJ = options.o main.o plot.o circular.o electro.o cylinder.o field.o

RUBBISH += *~ $(OBJ) $(NAME)

.PHONEY : all test show clean spotless install reconfigure

all : $(NAME).1 $(NAME)

test : show

TYPE = cylinder

show : $(TYPE).eps
	gv $(TYPE).eps

clean :
	$(RM) $(RUBBISH)

spotless : clean

#	$(RM) $(GGEN)

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTBINEX) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

reconfigure :
	cd ../.. ; ./configure ; cd src/vfplot

archive : archive-profile archive-testfields

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

ZPLOG = $(ADMIN)/profile/$(VERSION).gz

profile : $(ZPLOG)

gmon.out : cylinder.eps 

$(ZPLOG) : gmon.out
	$(GPROF) $(NAME) | gzip > $(ZPLOG)

archive-profile : $(ZPLOG)

RUBBISH += gmon.out

endif

# test plots

EPS = circular.eps electro2.eps electro3.eps cylinder.eps cylinder-ellipse.eps
PDF = circular.pdf electro2.pdf electro3.pdf cylinder.pdf 
RUBBISH += $(EPS) $(PDF) histogram.dat cylinder.sag cylinder.vgs

SIZE = -w4i

eps : $(EPS)

# illustrates scale-variance

circular.eps : $(NAME)
	./$(NAME) -d circular.dom -v -s0.2 -E -D1 -P0 \ #--break grid \
	          -p adaptive -t circular $(SIZE) -m 3/3/0 -o circular.eps

electro2.eps : $(NAME)
	./$(NAME) -v -D2 -s0.4 -f200 -P0.2m -t electro2 $(SIZE) \
	          -p adaptive -m 4/4/0 --overfill 6 -o electro2.eps

electro3.eps : $(NAME)
	./$(NAME) -v -D2 -s0.4 -P0.2m -f200 -t electro3 $(SIZE) --glyph triangle \
	          -p adaptive -m 4/4/0 --overfill 7 --length 0/1in -o electro3.eps

cylinder.eps cylinder.sag cylinder.vgs histogram.dat : $(NAME)
	./$(NAME) -v -f200 -D2 -s0.75 -t cylinder -m4/4/0 $(SIZE) -E \
		--dump-vectors cylinder.sag --histogram histogram.dat \
		-G cylinder.vgs -i50/10 -p adaptive -P0.0m -k5 -o cylinder.eps 

cylinder-ellipse.eps : $(NAME)
	./$(NAME) -v -f150 -D2 -s1 -t cylinder $(SIZE) \
	        -i30/10 -P0 -E -o cylinder-ellipse.eps 

.SUFFIXES : .eps .pdf

.eps.pdf :
	epstopdf $<

RUBBISH += testfields.log testfields.pdf

testfields.pdf : $(PDF) testfields.tex
	pdflatex testfields

archive-testfields : testfields.pdf
	install -m440 testfields.pdf $(ADMIN)/testfields/$(VERSION).pdf

# if you have configured with --enable-dmalloc the this has debugging
# info on memory usage, leaks etc

DMO = log-known,log-non-free,log-stats,error-free-null,log-trans
RUBBISH += dmalloc.log

dmalloc.log : 
	export DMALLOC_OPTIONS=$(DMO),log=dmalloc.log ; $(MAKE) cylinder.eps

# vector dump and plot - you need GMT for this

RUBBISH += cylinder.u.grd cylinder.v.grd cylinder-grd2.eps cylinder-sag.eps
CYLGRD = GMT xyz2grd -I0.015625/0.015625 -R-1/1/-1/1 -F

cylinder.u.grd cylinder.v.grd : cylinder.sag
	cut -f 1,2,3 cylinder.sag | $(CYLGRD) -Gcylinder.u.grd
	cut -f 1,2,4 cylinder.sag | $(CYLGRD) -Gcylinder.v.grd

cylinder-grd2.eps : cylinder.u.grd cylinder.v.grd
	./$(NAME) -v -f150 -D2 -s1 -i5/50 -p adaptive -P0 -E -o cylinder-grd2.eps \
		$(SIZE) cylinder.u.grd cylinder.v.grd

cylinder-sag.eps : cylinder.sag
	./$(NAME) -v -f150 -D2 -s1 -i5/50 -p adaptive -P0 -E -o cylinder-sag.eps \
		$(SIZE) cylinder.sag

# animation - you need imagemagick and mencoder for this

cylinder-png : $(NAME) Makefile
	./$(NAME) -v -D2 -s0.15 -t cylinder $(SIZE) --ellipse-pen 0.4m \
	          --animate -i200/5 --ke-drop 15 -P0 -E -o cylinder.eps
	gv cylinder.eps
	ls -1 anim.*.*.eps | xargs -P2 -n1 sh mkpng.sh 

cylinder.avi : cylinder-png
	mencoder 'mf://anim.*.png' -mf w=800:h=600:fps=25:type=png \
	  -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o cylinder.avi

cylinder.flv : cylinder-png
	perl anim-sequential.pl
	ffmpeg -i sqa-%4d.png -sameq -r 20 cylinder.flv

cylinder.avi : $(NAME) Makefile

ifdef OCTAVE

# this tests the matlab binary support, shear.m generates a 
# simple shear field

RUBBISH += shear.mat shear.vgs shear.eps

shear.mat : shear.m
	octave --silent shear.m

shear.eps : $(NAME) shear.mat
	./$(NAME) -v -f150 -D2 -s2e-4 -i500/5 -m 4/4/0 -p adaptive -P0 -E -o shear.eps \
		-G shear.vgs $(SIZE) shear.mat

endif