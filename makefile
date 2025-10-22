CC 		:= gcc

DEX_F 	:= -DEXPLODE
TIMEOUT	:= -DTIMEOUT

SDIR	:= src/
IPC_DIR := $(SDIR)ipc/
BIN_DIR := bin/

SRC 	:= $(SDIR)direttore.c $(SDIR)utente.c $(SDIR)operatore.c $(SDIR)erogatore_ticket.c
IPC 	:= $(IPC_DIR)message_queue.c $(IPC_DIR)semaphores.c $(IPC_DIR)shared_memory.c #$(IPC_DIR)signals.c
OBJ		:= $(SRC:$(SDIR)%.c=$(BIN_DIR)%.o)
OBJ_IPC := $(IPC:$(IPC_DIR)%.c=$(BIN_DIR)%.o)
TARGET  := $(BIN_DIR)direttore


.PHONY: all run clean   			#specifico che sono regole del makefile e non altro al di fuori

run: all
	./$(TARGET)

all: $(TARGET)
	$(CC) $(BIN_DIR)direttore.o $(OBJ_IPC) -o $(TARGET)
	$(CC) $(BIN_DIR)operatore.o $(OBJ_IPC) -o operatore.o 
	$(CC) $(BIN_DIR)erogatore_ticket.o $(OBJ_IPC) -o erogatore_ticket.o 
	$(CC) $(BIN_DIR)utente.o $(OBJ_IPC) -o utente.o

$(BIN_DIR)%.o: $(IPC_DIR)%.c 							#regola per compilare tutti i .c in .o  $@=target $< = dipendenze
	$(CC) -c $^ -o $@ 


$(BIN_DIR)%.o: $(SDIR)%.c 							#regola per compilare tutti i .c in .o  $@=target $< = dipendenze
	$(CC) -c $^ -o $@     	


clean:
	rm -f bin/*
	ipcrm
























