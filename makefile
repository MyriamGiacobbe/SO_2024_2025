CC = gcc

DEF ?= EXPLODE  #di default fa partire l'Explode, se voglio altro: make DEF=TIMEOUT
CFLAGS = -D$(DEF) -Wall -g -I src

BIN_DIR = bin
SRC_DIR = src
IPC_DIR = src/ipc

VPATH = $(SRC_DIR) $(IPC_DIR)

COMMON_OBJS =   $(BIN_DIR)/semaphores.o \
				$(BIN_DIR)/message_queue.o \
				$(BIN_DIR)/shared_memory.o 

TARGETS =	$(BIN_DIR)/direttore \
			$(BIN_DIR)/operatore \
			$(BIN_DIR)/utente \
			$(BIN_DIR)/erogatore

.PHONY: all clean run

run: all
	cd $(BIN_DIR) && ./direttore

all: $(BIN_DIR) $(TARGETS)

# Qui è come creare gli eseguibili dai .o
$(BIN_DIR)/direttore: $(BIN_DIR)/direttore.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/operatore: $(BIN_DIR)/operatore.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/utente: $(BIN_DIR)/utente.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/erogatore: $(BIN_DIR)/erogatore_ticket.o $(COMMON_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

# Qui è come creare i .o dai .c
$(BIN_DIR)/%.o: %.c 
		$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(TARGETS) $(BIN_DIR)/*.o
	ipcrm -a