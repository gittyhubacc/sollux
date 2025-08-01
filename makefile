CC := clang
CFLAGS := -g -O0 -Wno-int-to-void-pointer-cast -Wno-void-pointer-to-int-cast
LDFLAGS := -lcirces -lmf

BIN_NAME := sollux

SRC_DIR := ./src
BIN_DIR := ./bin

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%.c=%.o)

INSTALL_BIN := /usr/local/bin
BIN_TARGET = $(BIN_DIR)/$(BIN_NAME)


all: $(BIN_TARGET)

$(BIN_DIR)/$(BIN_NAME): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean install
clean:
	rm -f $(BIN_DIR)/$(BIN_NAME) $(OBJS)
	rmdir $(BIN_DIR)
install: $(BIN_TARGET)
	sudo cp $(BIN_TARGET) $(INSTALL_BIN)
