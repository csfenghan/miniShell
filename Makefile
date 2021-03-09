CC=gcc
CC_COMPILE=-g -I/home/fenghan/local/unix_lib
CC_LINK=-L/home/fenghan/local/unix_lib

SOURCE_FILES=main.c 
OBJ_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
OUTPUT=demo

all:$(OBJ_FILES)
	$(CC) $(CC_LINK) $^ -o $(OUTPUT)
	make -f makefile.user

%.o:%.c
	$(CC) -c $(CC_COMPILE) $^ -o $@

#####################################
.PHONY:clean 
clean:
	rm $(OBJ_FILES) $(OUTPUT)
