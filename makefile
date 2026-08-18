CC := gcc

BUILD := build
SRC   := src
TEST  := tests

CFLAGS := -Wall -Wextra -Wpedantic -I$(SRC)
LDFLAGS := 

INTERPRETER_SRC := $(wildcard $(SRC)/*.c)
TEST_SRC        := $(wildcard $(TEST)/*.c)
OBJS            := $(patsubst $(SRC)/%.c,$(BUILD)/$(SRC)/%.o,$(INTERPRETER_SRC))
TESTS           := $(patsubst $(TEST)/%.c,$(BUILD)/$(TEST)/%,$(TEST_SRC))
TARGET          := $(BUILD)/tea

.PHONY: create_build_dirs build build_tests test run debug

create_build_dirs:
	@mkdir -p $(BUILD)/$(SRC)
	@mkdir -p $(BUILD)/$(TEST)

$(OBJS): $(BUILD)/$(SRC)/%.o: $(SRC)/%.c
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

$(TESTS): $(OBJS)
$(TESTS): $(BUILD)/$(TEST)/%: $(TEST)/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(filter-out $(BUILD)/$(SRC)/main.o,$(OBJS))

.PHONY: build
build: create_build_dirs $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful"

build_tests: create_build_dirs $(TESTS)

test: build_tests
	@echo -e "testing\n"
	@for file in $(TESTS); do \
		echo "test $$file";    \
		$$file;                 \
	done

run: build
	@echo "Running target"
	@$(TARGET) $(ARGS)

debug: build
	@echo "debugging target"
	@$(DB) --args $(TARGET) $(ARGS)
