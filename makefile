CC := clang
CFLAGS := -g -O0 -Wno-int-to-void-pointer-cast -Wno-void-pointer-to-int-cast
LDFLAGS := -lcirces -lmf

BIN_NAME := sollux
BS_BIN_NAME := tmpc

SRC_DIR := ./src
BIN_DIR := ./bin

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%.c=%.o)

INSTALL_BIN := /usr/local/bin
BIN_SRC = $(BIN_DIR)/sollux.ta.c
BIN_OBJ = $(BIN_DIR)/sollux.ta.o
BIN_TARGET = $(BIN_DIR)/$(BIN_NAME)
BS_BIN := $(BIN_DIR)/$(BS_BIN_NAME)

all: $(BIN_TARGET)

$(BIN_TARGET): $(BS_BIN)
	./$(BS_BIN) <src/main.ta $(BIN_SRC)
	$(CC) $(CFLAGS) -c $(BIN_SRC) -o $(BIN_OBJ)
	$(CC) $(BIN_OBJ) $(LDFLAGS) -o $(BIN_TARGET)

$(BS_BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean install
clean:
	rm -f $(BS_BIN) $(OBJS)
	rmdir $(BIN_DIR)
install: $(BIN_TARGET)
	sudo cp $(BIN_TARGET) $(INSTALL_BIN)
