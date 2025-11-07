CC = gcc
explode: bin/direttore bin/operatore bin/utente bin/erogatore_ticket
#timeout: bin/direttore_t

CONFIG_EXPLODE = -DEXPLODE
CONFIG_TIMEOUTE = -DTIMEOUT

INCLUDE = src/*.h src/ipc/*.h

COMMON_DEPS = $(INCLUDE)

build/%.o: src/%.c $(COMMON_DEPS)
	$(CC) $(CONFIG_EXPLODE) -c $< -o $@

bin/direttore: build/direttore.o build/operatore.o build/utente.o build/erogatore_ticket.o $(COMMON_DEPS)
	$(CC) -o bin/direttore

bin/operatore: build/operatore.o $(COMMON_DEPS)
	$(CC) -o bin/operatore

bin/utente: build/utente.o $(COMMON_DEPS)
	$(CC) -o bin/dutente

bin/erogatore_ticket: build/erogatore_ticket.o $(COMMON_DEPS)
	$(CC) -o bin/erogatore_ticket

clean:
	rm -f build/* bin/*