CC := gcc

CFLAGS := -Wall -Wextra -Wpedantic
LDFLAGS := 

BUILD := build
SRC   := src

INTERPRETER_SRC := $(wildcard $(SRC)/*.c)
TARGET          := $(BUILD)/tea

.PHONY: build
build:
	@$(CC) $(CFLAGS) $(INTERPRETER_SRC) -o $(TARGET) $(LDFLAGS)

run: build
	@mkdir -p $(BUILD)
	@$(TARGET) $(ARGS)

debug: build
	@$(DB) --args $(TARGET) $(ARGS)
