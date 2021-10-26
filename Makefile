TARGET_EXEC:=main

# directory
BUILD_DIR:=./build
SRC_DIR:=./src
INC_DIRS:=./src

# files
# SRCS:=$(wildcard $(SRC_DIR)/**.c)
SRCS:=$(shell find $(SRC_DIR) -type f -iname '*.c')
OBJS:=$(SRCS:%=$(BUILD_DIR)/%.o)
INCS=$(foreach d, $(INC_DIRS), -I$d)

# compiler
# CROSS_COMPILE:=arm-linux-gnueabihf-
# CROSS_COMPILE:=aarch64-linux-gnu-
CC:=$(CROSS_COMPILE)gcc
C_FLAGS:=-Wall -std=gnu11 -Ofast $(INCS)
C_LINK:=-lm

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(C_FLAGS) $(OBJS) -o $@ $(C_LINK)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	@rm -f $(TARGET_EXEC)
	@rm -rf $(BUILD_DIR)
