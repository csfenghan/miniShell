CC=gcc
CC_FLAGES=-g -Werror -std=c11

SOURCE_FILES=main.c unix_api.c
OBJ_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
OUTPUT=miniShell

all:$(OBJ_FILES)
	$(CC) $^ -o $(OUTPUT)
	make -f makefile.user

%.o:%.c
	$(CC) -c $(CC_FLAGES) $^ -o $@

#####################################
.PHONY:clean 
clean:
	rm $(OBJ_FILES) $(OUTPUT)
