# Makefile for pwgrid
# (c) J.J.Green 2015

NAME = pwgrid

CC       = gcc
CFLAGS  += -I../../include -Wall -O3
LDFLAGS += -L../../lib
LDLIBS  += -lvfplot

default : all

GGEN = options.c options.h
OBJ  = options.o main.o pwgrid.o

RUBBISH += *~ $(OBJ) $(NAME) test.sag

.PHONY : all clean spotless test

all : $(NAME)

test : $(NAME)
	./$(NAME) -v -o test.sag 1 1 0.2 0.1

clean :
	$(RM) $(RUBBISH)

main.o : $(GGEN) main.c

options.o : CFLAGS += -Wno-unused-but-set-variable

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

$(GGEN) : options.ggo
	gengetopt -u -N -i options.ggo -f options -F options
