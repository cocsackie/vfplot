# Makefile for sag2grd
#
# J.J. Green 2008

NAME = sag2grd

default : all

all : $(NAME) $(NAME).1

RUBBISH += *~ .gmtcommands4 gmt.history

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

clean :
	$(RM) $(RUBBISH)

spotless : clean
	$(RM) $(NAME)

include ../Common.mk
include ../Accept.mk
include ../DocBook.mk
