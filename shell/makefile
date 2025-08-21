TARGET := shell
CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -fPIE -fno-omit-frame-pointer -ffreestanding -O2
LDFLAGS := -nostdlib -pie -Wl,-z,relro,-z,now --entry=main

SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo " [LINK] $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@echo " [CLEAN]"
	@rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild