CC := gcc
CFLAGS := -Wall -Wextra -pedantic -std=c11
LDFLAGS :=

ifeq ($(OS),Windows_NT)
CFLAGS += -DWIN32 -D_WIN32_WINNT=0x0600
LDFLAGS += -lws2_32
endif

BUILD_DIR := build
CLIENT_SRC := client-project/src/main.c
SERVER_SRC := server-project/src/main.c
CLIENT_BIN := $(BUILD_DIR)/client
SERVER_BIN := $(BUILD_DIR)/server

.PHONY: all client server run-client run-server clean

all: client server

client: $(CLIENT_BIN)

server: $(SERVER_BIN)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(CLIENT_BIN): $(CLIENT_SRC) client-project/src/protocol.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iclient-project/src $(CLIENT_SRC) -o $(CLIENT_BIN) $(LDFLAGS)

$(SERVER_BIN): $(SERVER_SRC) server-project/src/protocol.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iserver-project/src $(SERVER_SRC) -o $(SERVER_BIN) $(LDFLAGS)

run-client: client
	$(CLIENT_BIN)

run-server: server
	$(SERVER_BIN)

clean:
	rm -rf $(BUILD_DIR)
