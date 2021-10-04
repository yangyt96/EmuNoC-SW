TARGET_EXEC:=main

# directory
BUILD_DIR:=./build
SRC_DIR:=./src

# files
# SRCS:=$(wildcard $(SRC_DIR)/**.c)
SRCS:=$(shell find $(SRC_DIR) -type f -iname '*.c')
OBJS:=$(SRCS:%=$(BUILD_DIR)/%.o)

# compiler
# CROSS_COMPILE:=arm-linux-gnueabihf-
CC:=$(CROSS_COMPILE)gcc
C_FLAGS:=-Wall -std=gnu11

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(C_FLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	@rm -f $(TARGET_EXEC)
	@rm -rf $(BUILD_DIR)

see:
	echo $(SRCS)

test:
	gcc $(C_FLAGS) src/main.c src/xaxidma_print.c -o main
	./main