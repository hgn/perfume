CC = gcc
CFLAGS = -Wall -ggdb -Wextra -Werror -fstack-protector-strong -fPIE -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer -Iinclude \
         -Wformat -Wformat-security -fstack-clash-protection -fcf-protection -falign-functions=32 -fdiagnostics-color=always -frecord-gcc-switches -pipe
# used for the (unit) test target
SANITIZE_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS = -Wl,-z,relro,-z,now -Wl,-z,noexecstack -shared -fPIC
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release
TEST_DIR = test

TARGET = libperfume.so
DEBUG_TARGET = $(DEBUG_DIR)/$(TARGET)
RELEASE_TARGET = $(RELEASE_DIR)/$(TARGET)

INCLUDE_TARGET = /usr/include
LIB_TARGET = /usr/lib

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
	install -m 644 $(INCLUDE_DIR)/perfume.h $(INCLUDE_TARGET)
	install -m 755 $(RELEASE_TARGET) $(LIB_TARGET)
	ldconfig

clean:
	rm -rf $(BUILD_DIR) $(TEST_DIR)/test_perfume $(TEST_DIR)/example

test-build: CFLAGS += $(SANITIZE_FLAGS)
test-build: LDFLAGS += $(SANITIZE_FLAGS)
test-build: $(RELEASE_TARGET)
	$(CC) $(CFLAGS) -o $(TEST_DIR)/test_perfume $(TEST_DIR)/test_perfume.c -L$(DEBUG_DIR) -lperfume

test: test-build
	$(test-build)
	LD_LIBRARY_PATH=$(DEBUG_DIR) $(TEST_DIR)/test_perfume

example: $(RELEASE_TARGET)
	$(CC) $(CFLAGS) -o $(TEST_DIR)/example $(TEST_DIR)/example.c -L$(RELEASE_DIR) -lperfume

example-run: example
	LD_LIBRARY_PATH=$(RELEASE_DIR) $(TEST_DIR)/example

.PHONY: all debug release test test-build example example-run clean install

