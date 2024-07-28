CC = gcc
CFLAGS = -Wall -Wextra -Werror -fstack-protector-strong -fPIE -D_FORTIFY_SOURCE=2 -Iinclude
SANITIZE_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS = -Wl,-z,relro,-z,now -shared -fPIC
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release
TEST_DIR = test

TARGET = libperfume.so
DEBUG_TARGET = $(DEBUG_DIR)/$(TARGET)
RELEASE_TARGET = $(RELEASE_DIR)/$(TARGET)
INCLUDE_TARGET = /usr/local/include
LIB_TARGET = /usr/local/lib

SRC = $(SRC_DIR)/perfume.c
OBJ = $(SRC:.c=.o)

all: debug release

debug: CFLAGS += -g -fno-omit-frame-pointer 
debug: $(DEBUG_TARGET)

release: CFLAGS += -O2
release: $(RELEASE_TARGET)

$(DEBUG_TARGET): $(SRC)
	mkdir -p $(DEBUG_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(RELEASE_TARGET): $(SRC)
	mkdir -p $(RELEASE_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: $(RELEASE_TARGET)
	mkdir -p $(INCLUDE_TARGET)
	cp $(INCLUDE_DIR)/perfume.h $(INCLUDE_TARGET)
	cp $(RELEASE_TARGET) $(LIB_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TEST_DIR)/test_perfume

test-build: CFLAGS += $(SANITIZE_FLAGS)
test-build: LDFLAGS += $(SANITIZE_FLAGS)
test-build: $(RELEASE_TARGET)
	$(CC) $(CFLAGS) -o $(TEST_DIR)/test_perfume $(TEST_DIR)/test_perfume.c -L$(RELEASE_DIR) -lperfume

test: test-build
	$(test-build)
	LD_LIBRARY_PATH=$(RELEASE_DIR) $(TEST_DIR)/test_perfume

.PHONY: all debug release test test-build

