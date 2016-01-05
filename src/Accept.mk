# acceptance tests

TESTD = ../fixtures
TESTX = ../accept

accept :
	./accept.sh $(NAME) $(TESTX) $(TESTD)

.PHONY : accept
