CC = gcc

DEF ?= EXPLODE  #di default fa partire l'Explode, se voglio altro: make DEF=TIMEOUT
CFLAGS = -D$(DEF) -Wall -g 

BIN_DIR = bin
SRC_DIR = src
IPC_DIR = src/ipc

COMMON_OBJS =   $(BIN_DIR)/semaphores.o \
				$(BIN_DIR)/message_queue.o \
				$(BIN_DIR)/shared_memory.o 

TARGETS =	$(BIN_DIR)/direttore \
			$(BIN_DIR)/operatore \
			$(BIN_DIR)/utente \
			$(BIN_DIR)/erogatore

.PHONY: all clean run

run: all
	cd ./bin && ./direttore

all: $(TARGETS)

# Qui è come creare gli eseguibili dai .o
$(BIN_DIR)/direttore: $(BIN_DIR)/direttore.o $(COMMON_OBJS)
		$(CC) $^ -o $@

$(BIN_DIR)/operatore: $(BIN_DIR)/operatore.o $(COMMON_OBJS)
		$(CC) $^ -o $@

$(BIN_DIR)/utente: $(BIN_DIR)/utente.o $(COMMON_OBJS)
		$(CC) $^ -o $@

$(BIN_DIR)/erogatore: $(BIN_DIR)/erogatore_ticket.o $(COMMON_OBJS)
		$(CC) $^ -o $@

# Qui è come creare i .o dai .c
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
		$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(IPC_DIR)/%.c
		$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(TARGETS) $(BIN_DIR)/*.o