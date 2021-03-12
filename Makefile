CC=gcc
CC_COMPILE=-g 
CC_LINK= -lunix_lib -lpthread

SOURCE_FILES=main.c 
OBJ_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
OUTPUT=demo

all:$(OBJ_FILES)
	$(CC) $^ -o $(OUTPUT) $(CC_LINK) 
	make -f makefile.user

%.o:%.c
	$(CC) -c $(CC_COMPILE) $^ -o $@

#####################################
.PHONY:clean 
clean:
	rm $(OBJ_FILES) $(OUTPUT)
