# Makefile for pwgrid
# (c) J.J.Green 2015

NAME = pwgrid

CC       = gcc
CFLAGS  += -std=c99 -I../../include -Wall -O3
LDFLAGS += -L../../lib
LDLIBS  += -lvfplot -lm

default : all

GGEN = options.c options.h
OBJ  = options.o main.o pwgrid.o

RUBBISH += *~ $(OBJ) $(NAME) test.dat

.PHONY : all clean spotless test

all : $(NAME)

test : $(NAME)
	./$(NAME) -v -o test.dat -n 256 1 1e-4 0.2 0.2

clean :
	$(RM) $(RUBBISH)

main.o : $(GGEN) main.c

options.o : CFLAGS += -Wno-unused-but-set-variable

$(NAME) : $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(NAME)

$(GGEN) : options.ggo
	gengetopt -u -N -i options.ggo -f options -F options
