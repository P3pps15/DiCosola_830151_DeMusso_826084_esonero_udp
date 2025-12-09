# Makefile for UDP Client-Server Project
# Supports Windows (MinGW), Linux, and macOS

# Detect OS
# First check if OS is already set (Windows sets OS=Windows_NT)
ifeq ($(OS),Windows_NT)
    # Windows detected via OS environment variable
    UNAME_S := Windows
else
    # Try to detect via uname
    UNAME_S := $(shell uname -s 2>/dev/null || echo "Unknown")
    # Check for Windows (MinGW/MSYS)
    ifneq ($(findstring MINGW,$(UNAME_S)),)
        OS := Windows_NT
    endif
    ifneq ($(findstring MSYS,$(UNAME_S)),)
        OS := Windows_NT
    endif
    ifneq ($(findstring CYGWIN,$(UNAME_S)),)
        OS := Windows_NT
    endif
    # If uname fails, might be Windows
    ifeq ($(UNAME_S),Unknown)
        # Try to detect via compiler
        ifneq ($(shell gcc -dumpmachine 2>/dev/null | grep -i mingw),)
            OS := Windows_NT
            UNAME_S := Windows
        endif
    endif
endif

# Debug directory
DEBUG_DIR = debug

# Compiler and flags
ifeq ($(OS),Windows_NT)
    # Windows with MinGW
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11 -DWIN32
    SERVER_TARGET = $(DEBUG_DIR)/server-project.exe
    CLIENT_TARGET = $(DEBUG_DIR)/client-project.exe
    LDFLAGS_SERVER = -lws2_32
    LDFLAGS_CLIENT = -lws2_32
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = if not exist
else ifeq ($(UNAME_S),Linux)
    # Linux
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11
    SERVER_TARGET = $(DEBUG_DIR)/server-project
    CLIENT_TARGET = $(DEBUG_DIR)/client-project
    LDFLAGS_SERVER = 
    LDFLAGS_CLIENT = 
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
else ifeq ($(UNAME_S),Darwin)
    # macOS
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11
    SERVER_TARGET = $(DEBUG_DIR)/server-project
    CLIENT_TARGET = $(DEBUG_DIR)/client-project
    LDFLAGS_SERVER = 
    LDFLAGS_CLIENT = 
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
else
    # Default to Linux settings
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c11
    SERVER_TARGET = $(DEBUG_DIR)/server-project
    CLIENT_TARGET = $(DEBUG_DIR)/client-project
    LDFLAGS_SERVER = 
    LDFLAGS_CLIENT = 
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
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
ifeq ($(OS),Windows_NT)
	@if not exist $(DEBUG_DIR) mkdir $(DEBUG_DIR)
else
	@$(MKDIR) $(DEBUG_DIR)
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
ifeq ($(OS),Windows_NT)
	-$(RM) $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ) 2>nul
	-if exist $(DEBUG_DIR) $(RMDIR) $(DEBUG_DIR) 2>nul
else
	-$(RM) $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJ) $(CLIENT_OBJ)
	-$(RMDIR) $(DEBUG_DIR) 2>/dev/null || true
endif
	@echo "Clean completed."

# Phony targets
.PHONY: all server client clean $(DEBUG_DIR)

