CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS_SERVER = -lmysqlclient
BUILD_DIR = build
SRC_DIR = src

.PHONY: all clean server client

all: server client

# Server build
server: $(BUILD_DIR)/server

$(BUILD_DIR)/server: $(SRC_DIR)/server/server.c include/common.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_SERVER)

# Client build
client: $(BUILD_DIR)/client

$(BUILD_DIR)/client: $(SRC_DIR)/client/client.c include/common.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)