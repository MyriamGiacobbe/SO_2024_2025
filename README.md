# Progetto per il corso di Sistemi Operativi

# Obiettivo
Il progetto ha lo scopo di simulare il funzionamento di un ufficio postale composto da
utenti, operatori, un processo erogatore e un processo direttore che coordina l’intera ese-
cuzione. La simulazione è realizzata mediante la creazione di diversi processi che comuni-
cano e si sincronizzano utilizzando le principali strutture IPC, come memoria condivisa,
semafori, code di messaggi, segnali e FIFO.
L’obiettivo è riprodurre realisticamente il comportamento di un sistema complesso, do-
ve più attori interagiscono in modo concorrente, dove gli utenti richiedono servizi, gli ope-
ratori li erogano e il direttore gestisce il ciclo delle giornate, assicurando un coordinamento
corretto delle risorse. Alla fine del progetto viene prodotta una relazione.

# Strumenti
Per la realizzazione del progetto sono stati impiegati numerosi meccanismi IPC, ciascuno
scelto per la sua specifica caratteristica.

1. Memoria condivisa
      La memoria condivisa contiene strutture dati globali, accessibili da tutti i processi. Per
      garantire correttezza, l’accesso ad alcune aree è protetto da semafori che impediscono
      condizioni di competizione.
2. I semafori
      Svolgono un ruolo essenziale:
      • sincronizzazione di inizio e fine giornata tramite barriere;
      • gestione degli sportelli disponibili per ogni servizio;
      • mutua esclusione sugli aggiornamenti delle statistiche.
4. Code di messaggi
      Le message queue permettono la comunicazione bloccante tra utenti, operatore ed ero-
      gatore. La separazione tramite tipi di messaggio consente agli operatori di ricevere solo
      le richieste relative al proprio servizio e agli utenti di ricevere solo le richieste relative al
      proprio pid.
5. Segnali
      Il direttore utilizza i segnali per comunicare eventi globali, come la fine di una giornata
      simulata (SIGUSR1) o la fine dell’intera simulazione (SIGTERM). I processi gestiscono tali
      segnali tramite handler dedicati, garantendo una terminazione ordinata.
6. FIFO
      La FIFO permette il collegamento tra la simulazione principale e il programma esterno
      add_users, garantendo la possibilità di aggiungere nuovi utenti in qual si voglia punto
      della simulazione.

# Statistiche
Una parte importante del progetto consiste nella raccolta delle statistiche, che includono:
• numero di utenti serviti per giornata e per servizio;
• numero di richieste soddisfatte o non soddisfatte;
• durata media dei servizi;
• carico di lavoro degli operatori;
• numero di pause effettuate.
Al termine di ciascuna giornata, il direttore salva i valori in un file CSV, che può essere
successivamente utilizzato per l’analisi dei risultati.
