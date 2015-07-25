# Makefile for sag2grd
#
# J.J. Green 2008

NAME = sag2grd

default : all

all : $(NAME) $(NAME).1

RUBBISH += *~ $(NAME)

$(NAME) : $(NAME).pl
	echo "#! /usr/bin/perl -w" > $(NAME)
	echo 'our $$version = "$(VERSION)";' >> $(NAME)
	cat $(NAME).pl >> $(NAME)
	chmod 755 $(NAME)

FIXTURES = ../fixtures
GRD = u.grd v.grd
RUBBISH += $(GRD)

test : $(GRD)

$(GRD) : $(NAME)
	./$(NAME) -v -i $(FIXTURES)/test.sag $(GRD)

install : all
	$(INSTDIR) $(EXEC)
	$(INSTDIR) $(MAN1)
	$(INSTSCRIPT) $(NAME) $(EXEC)/$(NAME)
	$(INSTDATA) $(NAME).1 $(MAN1)/$(NAME).1

clean spotless :
	$(RM) $(RUBBISH)

include ../Common.mk
