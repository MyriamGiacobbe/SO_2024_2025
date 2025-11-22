CC = gcc

DEF ?= TIMEOUT  #di default fa partire l'Explode, se voglio altro: make DEF=TIMEOUT
CFLAGS = -D$(DEF) -Wall -g -I src

BIN_DIR = bin
BUILD_DIR = build
SRC_DIR = src
IPC_DIR = src/ipc

COMMON_OBJS =   $(BUILD_DIR)/semaphores.o\
				$(BUILD_DIR)/message_queue.o \
				$(BUILD_DIR)/shared_memory.o 

TARGETS =	$(BIN_DIR)/direttore \
			$(BIN_DIR)/operatore \
			$(BIN_DIR)/utente \
			$(BIN_DIR)/erogatore

.PHONY: all clean run explode timeout dirs

explode: clean
	$(MAKE) all DEF=EXPLODE

timeout: clean
	$(MAKE) all DEF=TIMEOUT

all: dirs $(TARGETS)

run: all
	cd $(BIN_DIR) && ./direttore

dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)


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

$(BUILD_DIR)/%.o: $(IPC_DIR)/%.c 
		$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(TARGETS) $(BUILD_DIR)/*.o
	-ipcrm -a