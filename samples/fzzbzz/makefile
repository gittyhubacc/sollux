CC := clang
CFLAGS := -g -O0
LDFLAGS := -lcirces -lmf

OBJ_LANG_NAME := fzzbzz

# where is sollux coming from
SOLLUX_BIN := sollux
# the definition of the language
OBJ_COMPILER_DEF := $(OBJ_LANG_NAME).ta
# the filename of the object compiler's source
OBJ_COMPILER_SRC := $(OBJ_LANG_NAME)c.c
OBJ_COMPILER_OBJ := $(OBJ_COMPILER_SRC:%.c=%.o)
OBJ_COMPILER := $(OBJ_COMPILER_OBJ:%.o=%)
# the filename of source written in the object language
OBJ_SRC := program.$(OBJ_LANG_NAME)
# an obj program written in the object lang
OBJ_OBJ := $(basename $(OBJ_SRC))

all: $(OBJ_OBJ)

$(OBJ_COMPILER_SRC): $(OBJ_COMPILER_DEF)
	$(SOLLUX_BIN) <$< $@

$(OBJ_COMPILER_OBJ): $(OBJ_COMPILER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_COMPILER): $(OBJ_COMPILER_OBJ)
	$(CC) $(OBJ_COMPILER_OBJ) $(LDFLAGS) -o $@

$(OBJ_OBJ): $(OBJ_SRC) $(OBJ_COMPILER)
	./$(OBJ_COMPILER) <$(OBJ_SRC) $@

.PHONY: clean
clean:
	rm -f $(OBJ_COMPILER) $(OBJ_COMPILER_OBJ) $(OBJ_COMPILER_SRC) $(OBJ_OBJ)
