include makefile.defines

SRC_FILES=main.c \
	job_manger.c \
	signal_handler.c \
	parser_args.c \
	builtin_command.c
OBJ_FILES=$(patsubst %.c,%.o,$(SRC_FILES))
OUTPUT=miniShell

###########################################
#	构建过程
###########################################

all:$(OUTPUT)
	@echo "successfully build"


$(OUTPUT):$(OBJ_FILES)
	$(V) $(LD) $^ -o $@ $(LD_FLAGS)

%.o:%.c
	$(V) $(CC) -c  $^ -o $@ $(CC_FLAGS)
#############################################
#	伪指令
############################################
.PHONY:clean run
clean:
	rm $(OBJ_FILES) $(OUTPUT)

.PHONY:
	./$(OUTPUT)
