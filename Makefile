# Makefile for UDP Client-Server Project
# Supports Windows (MinGW), Linux, and macOS

# Detect OS
# Check for MSYS/MINGW first (uses bash, not cmd)
ifdef MSYSTEM
    IS_WINDOWS = 1
    IS_MSYS = 1
else
    # Windows sets OS=Windows_NT environment variable
    ifdef OS
        ifeq ($(OS),Windows_NT)
            IS_WINDOWS = 1
            IS_MSYS = 0
        else
            IS_WINDOWS = 0
            IS_MSYS = 0
        endif
    else
        # Check for Windows via COMSPEC
        ifdef COMSPEC
            IS_WINDOWS = 1
            IS_MSYS = 0
        else
            # Default to Unix-like
            IS_WINDOWS = 0
            IS_MSYS = 0
        endif
    endif
endif

# Debug directory
DEBUG_DIR = debug

# Compiler and flags
ifeq ($(IS_WINDOWS),1)
    # Windows with MinGW
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11 -DWIN32
    SERVER_TARGET = $(DEBUG_DIR)/server-project.exe
    CLIENT_TARGET = $(DEBUG_DIR)/client-project.exe
    LDFLAGS_SERVER = -lws2_32
    LDFLAGS_CLIENT = -lws2_32
    ifeq ($(IS_MSYS),1)
        # MSYS/MINGW uses Unix commands
        RM = rm -f
        RMDIR = rm -rf
    else
        # Windows CMD uses Windows commands
        RM = del /Q
        RMDIR = rmdir /S /Q
    endif
else
    # Linux/macOS/Unix
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11
    SERVER_TARGET = $(DEBUG_DIR)/server-project
    CLIENT_TARGET = $(DEBUG_DIR)/client-project
    LDFLAGS_SERVER = 
    LDFLAGS_CLIENT = 
    RM = rm -f
    RMDIR = rm -rf
endif

# Source files
SERVER_SRC = server-project/src/main.c
CLIENT_SRC = client-project/src/main.c

# Object files
SERVER_OBJ = server-project/src/main.o
CLIENT_OBJ = client-project/src/main.o

# Default target
all: server client

# Create debug directory
$(DEBUG_DIR):
ifeq ($(IS_WINDOWS),1)
ifeq ($(IS_MSYS),1)
	@mkdir -p $(DEBUG_DIR)
else
	@if not exist $(DEBUG_DIR) mkdir $(DEBUG_DIR)
endif
else
	@mkdir -p $(DEBUG_DIR)
endif

# Server target
server: $(DEBUG_DIR) $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SRC) server-project/src/protocol.h | $(DEBUG_DIR)
	@echo "Compiling server..."
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC) $(LDFLAGS_SERVER)

# Client target
client: $(DEBUG_DIR) $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC) client-project/src/protocol.h | $(DEBUG_DIR)
	@echo "Compiling client..."
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(LDFLAGS_CLIENT)

# Clean target
clean:
	@echo "Cleaning..."
ifeq ($(IS_WINDOWS),1)
ifeq ($(IS_MSYS),1)
	-$(RM) $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ) 2>/dev/null || true
	-$(RMDIR) $(DEBUG_DIR) 2>/dev/null || true
else
	-$(RM) $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ) 2>nul
	-if exist $(DEBUG_DIR) $(RMDIR) $(DEBUG_DIR) 2>nul
endif
else
	-$(RM) $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ)
	-$(RMDIR) $(DEBUG_DIR) 2>/dev/null || true
endif
	@echo "Clean completed."

# Phony targets
.PHONY: all server client clean $(DEBUG_DIR)

