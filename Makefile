CC=gcc
CC_FLAGES=-g

SOURCE_FILES=main.c
OBJ_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
OUTPUT=miniShell

all:$(OBJ_FILES)
	$(CC) $^ -o $(OUTPUT)

%.o:%.c
	$(CC) -c $(CC_FLAGES) $^ -o $@

#####################################
.PHONY:clean 
clean:
	rm $(OBJ_FILES) $(OUTPUT)
