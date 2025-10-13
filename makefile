CC = gcc -DEXPLODE         # Compiler used is gcc
RM = rm -f        # Command to remove files
CFLAGS = -Wall -Wextra -std=c99 -I./src/ipc   # Compilation flags and include path

# Directory paths
SRC_DIR = src
IPC_DIR = $(SRC_DIR)/ipc
BIN_DIR = bin

# Common IPC object files
IPC_OBJS = \
	$(IPC_DIR)/message_queue.o \
	$(IPC_DIR)/semaphores.o \
	$(IPC_DIR)/shared_memory.o \
	$(IPC_DIR)/signals.o

# Individual process source files
DIRETTORE_SRC = $(SRC_DIR)/direttore.c
UTENTE_SRC = $(SRC_DIR)/utente.c
OPERATORE_SRC = $(SRC_DIR)/operatore.c
EROGATORE_SRC = $(SRC_DIR)/erogatore_ticket.c

# Corresponding executables
DIRETTORE_EXE = $(BIN_DIR)/direttore
UTENTE_EXE = $(BIN_DIR)/utente
OPERATORE_EXE = $(BIN_DIR)/operatore
EROGATORE_EXE = $(BIN_DIR)/erogatore_ticket

# Default target
all: $(DIRETTORE_EXE) $(UTENTE_EXE) $(OPERATORE_EXE) $(EROGATORE_EXE)

run: $(DIRETTORE_EXE)
	./$(DIRETTORE_EXE)

# Linking rules
$(DIRETTORE_EXE): $(DIRETTORE_SRC) $(IPC_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(UTENTE_EXE): $(UTENTE_SRC) $(IPC_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OPERATORE_EXE): $(OPERATORE_SRC) $(IPC_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(EROGATORE_EXE): $(EROGATORE_SRC) $(IPC_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Generic rule to build IPC object files
$(IPC_DIR)/%.o: $(IPC_DIR)/%.c $(IPC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean compiled files
clean:
	$(RM) $(IPC_OBJS)
	$(RM) $(BIN_DIR)/*
