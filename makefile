CC = gcc

DEF ?= EXPLODE  #di default fa partire l'Explode, se voglio altro: make DEF=TIMEOUT
CFLAGS = -D$(DEF) -Wall -g -I src

BIN_DIR = bin
BUILD_DIR = build
SRC_DIR = src
IPC_DIR = src/ipc

VPATH = $(SRC_DIR) $(IPC_DIR)

COMMON_OBJS =   $(IPC_DIR)/semaphores.c \
				$(IPC_DIR)/message_queue.c \
				$(IPC_DIR)/shared_memory.c 

TARGETS =	$(BIN_DIR)/direttore \
			$(BIN_DIR)/operatore \
			$(BIN_DIR)/utente \
			$(BIN_DIR)/erogatore

.PHONY: all clean run

run: all
	cd $(BIN_DIR) && ./direttore

all: $(BIN_DIR) $(TARGETS)

# Qui è come creare gli eseguibili dai .o
$(BIN_DIR)/direttore: $(BUILD_DIR)/direttore.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/operatore: $(BUILD_DIR)/operatore.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/utente: $(BUILD_DIR)/utente.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/erogatore: $(BUILD_DIR)/erogatore_ticket.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

# Qui è come creare i .o dai .c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c 
		$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(TARGETS) $(BIN_DIR)/*.o
	rm -f $(BUILD_DIR)/*.o
	ipcrm -a