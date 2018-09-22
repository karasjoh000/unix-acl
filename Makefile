SRC=src
INC=include
FLAGS=-g
get: $(SRC)/main.c $(SRC)/acl.c $(SRC)/debug.c $(SRC)/get.c $(SRC)/getregex.c
	gcc -I$(SRC)/$(INC) $(SRC)/main.c $(SRC)/acl.c $(SRC)/debug.c $(SRC)/get.c $(SRC)/getregex.c $(FLAGS) -o get
clean:
	rm get
