NAME = vfplot

CFLAGS  += -I../include -DVERSION=\"$(VERSION)\"
LDFLAGS += -L../lib -L.
LDLIBS  += -lfield -lvfplot -lkdtree

ADMIN    = ../../..
FIXTURES = ../fixtures

default : all

include ../Common.mk
include ../Accept.mk

GGEN = options.c options.h
FOBJ = field.o field_sag.o field_gfs.o field_grd2.o field_mat.o \
       circular.o electro.o cylinder.o
OBJ  = options.o main.o plot.o

RUBBISH += libfield.a $(OBJ) $(FOBJ) $(NAME)

.PHONY : all test show clean spotless install

all : $(NAME).1 $(NAME)

test : show

TYPE = cylinder

show : $(TYPE).eps
	gv $(TYPE).eps

clean :
	$(RM) $(RUBBISH)

spotless : clean

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTBINEX) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

archive : archive-profile archive-testfields


# program

$(NAME) : $(OBJ) libfield.a
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

main.o : $(GGEN) main.c

libfield.a : $(FOBJ)
	ar rs libfield.a $(FOBJ)

options.o : CFLAGS += -Wno-unused-but-set-variable

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
	./$(NAME) -d $(FIXTURES)/circular.dom -v -s0.2 -E -D1 -P0 \
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

%.pdf : %.eps
	epstopdf $<

RUBBISH += testfields.log testfields.pdf testfields.aux

testfields.pdf : $(PDF) testfields.tex
	pdflatex testfields

archive-testfields : testfields.pdf
	install -m440 testfields.pdf $(ADMIN)/testfields/$(VERSION).pdf


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
