include makefile.defines

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
