*english version below*
## Progetto di Sistemi Operativi 2023/2024

## Componenti del gruppo:

- 1 Arianna Lezzi : 947010
- 2 Christian Lombardozzi : 932948
- 3 Matteo Naglieri : 930702

## Comandi base per lanciare il progetto:

- il comando "make" sul terminale viene eseguita la compilazione dei vari file, inoltre viene eseguito anche un linking
  al file di libreria "GeneralsUtils". I file oggetto sono salvati nella cartella "bin"
- il comando "make start" si fa eseguire il processo master
- il comando "make clean" rimuove i file oggetto creati

## Processi principali del progetto:

- Master
- Atomo
- Alimentazione
- Attivatore

.
.
.

## Master

Il processo master gestisce i ritmi delle simulazioni, procede subito alla lettura dei parametri delle configurazioni
contenuti nei file .txt allegati, successivamente crea una memoria condivisa che conterrà:
i valori iniziali della simulazione, gli id del set di semafori, della coda di messaggi e della memoria condivisa
utilizzata per la raccolta dei dati utili alle stampe della simulazione (Dump).
Successivamente si occupa della creazione iniziale degli altri tre tipi di processi attraverso una fork() che al suo
interno,
tramite una execve(), passa il codice contenuto nei file .c dei rispettivi processi e il buffer con l'indirizzo alla
memoria condivisa contenente gli id elencati in precedenza, utili per le varie attività dei processi.
Attesa la corretta inizializzazione degli altri processi creati, attraverso un semaforo (quando questo raggiunge lo
zero)
inizia l'effettiva simulazione stampando ogni secondo i valori dei dati richiesti presenti in memoria condivisa, sempre
grazie all'utiizzo di un set di semafori, 8 in totale, per la lettura di parametri specifici del dump, consentendo il
massimo parallellismo dei processi.
Ad ogni ciclo di stampa controlla che non si sia verificato nessun caso di "terminazione", se così fosse procede a
mandare segnali ai processi per i vari detach dalle strutture condivise, successivamente anche il master procede al
detach,
andando tutto a buon fine stampa la motivazione della fine della simulazione e quest'ultima termina.

## Atomo

Il processo atomo viene creato in tre possibili situazioni: dal Master, dall'Alimentazione e dall'Atomo stesso

- Il processo atomo viene creato in prima istanza dal Master, il quale crea un numero pari a: N_ATOMI_INIT
  di atomi, il master passa attraverso una pipe un numero atomico generato casualmente tra 1 e N_ATOM_MAX.
  Il figlio inserisce negli argomenti passati al execve l'id della memoria condivisa e il numero atomico passatogli.
- Il processo Atomo viene creato in seconda istanza dall'Alimentazione che ogni STEP_ALIMENTAZIONE immette un certo
  numero di
  nuovi atomi, anche a questi atomi vengono passati i numeri atomici tramite pipe.
- Il processo in ultima istanza viene creato dall'atomo stesso quando leggendo
  sulla coda di messaggi trova un messaggio di scissione da parte dell'Attivatore.
  Se trova un messaggio il cui corpo del testo sia l'intero 1, lo interpreta come comando di scissione,
  proverà quindi a eseguire una fork. La fork per scissione avviene solo se il numero
  atomico del processo(Atomo) è maggiore o uguale a una costante MIN_NUM_ATOMICO, definita in fase di configurazione.
  In caso positivo viene effettuata la scissione e viene liberata energia secondo la politica stabilita, inoltre il
  padre si occupa di
  passare al figlio un numero atomico, ricavato dimezzando il proprio.
  In caso negativo l'Atomo diventa una scoria e termina.

Tutte queste operzioni aggiornano il Dump con accesso in mutua esclusione alle variabili interessate.

## Alimentazione

il processo Alimentazione viene creato dal master e si occupa di creare N_NUOVI_ATOMI ogni STEP_ALIMENTAZIONE
nanosecondi,
abbiamo gestito questa temporizzazione tramite una nanosleep.
Ogni volta che l'alimentazione crea un nuovo atomo, aggiorna le variabili che tengono traccia del conteggio degli atomi
e del numero di alimentazioni prodotte
in quel secondo tramite i due semafori ad esse correlate.
Inoltre, il processo alimentazione quando crea un nuovo atomo si occupa di assegnarli il numero atomico con lo stesso
meccanismo
di assegnazione descritto nel processo Master.

## Attivatore

Il processo attivatore, una volta creato dal master, procede all'inizializzazione, ovvero ad attaccarsi alla memoria
condivisa
per ricavare gli id della coda di messaggi e del dump dati, utili al suo corretto funzionamento, successivamnete aspetta
che
tutti i vari processi siano inizializzati con un semaforo (attende che arrivi a 0).
Quando parte la simulazione si occupa di inserie ogni STEP ATTIVATORE 10 messaggi con msg_txt = 1 nella coda, che quando
letti dagli
atomi gli daranno il comando di compiere una scissione.
Quando il processo riceve il segnale di terminazione il suo handler si occupera invece di inserire uno "0" nella coda
di messaggi, che quando letto, darà invece il comando all'atomo di terminare.

## Terminazione

Come spiegato in precedenza la terminazione è coadiuvata dal Master, il quale prima di rimuovere
tutte le strutture condivise, si occupa di mandare dei segnali (SIGTERM) ai suoi processi, in particolare
all'Attivatore e all'Alimentazione. Gli Atomi ricevono il messaggio di terminazione dall'Attivatore
che pone un messaggio con corpo pari a 0 nella coda. Gli atomi quando leggono questo messaggio, prima di terminare, si
occupano di inserire un nuovo messaggio di terminazione per il prossimo Atomo che leggerà dalla coda di
messaggi.


## Operating Systems Project 2023/2024


## Group Members
- 1 Arianna Lezzi : 947010
- 2 Christian Lombardozzi : 932948
- 3 Matteo Naglieri : 930702


## Basic Commands to Run the Project
- make: compiles all the source files and performs the linking with the library file GeneralsUtils. The generated object files are stored in the bin directory
- make start: executes the Master process
- make clean: removes all generated object files

## Main Processes in the Project
- Master
- Atom
- Feeder
- Activator


## Master
The Master process coordinates the simulation workflow. It first reads the configuration parameters from the provided .txt files. Then, it creates a shared memory segment that contains:
- The initial simulation values
- The IDs of the semaphore set, the message queue, and the shared memory used for collecting simulation data (Dump)
After that, the Master creates the other three types of processes through a fork(). Inside the forked process, an execve() call is used to execute the corresponding .c source file, along with a buffer containing the IDs of the shared resources mentioned above, which are required by the processes.
Once all processes are correctly initialized (synchronized via a semaphore reaching zero), the simulation begins. The Master prints the simulation data every second, reading them from shared memory using a total of 8 semaphores to guarantee maximum parallelism during the dump operations.
At each print cycle, the Master checks whether a termination condition has occurred. If so, it sends signals to the processes to detach from the shared structures. Finally, the Master itself detaches, prints the reason for termination, and ends the simulation.


## Atom
The Atom process can be created in three different ways: by the Master, by the Feeder, or by another Atom itself.
#### 1. Created by the Master:
At startup, the Master spawns N_ATOMI_INIT atoms. For each atom, the Master generates a random atomic number between 1 and N_ATOM_MAX and sends it through a pipe. The child then starts via execve(), receiving the shared memory ID and its atomic number as arguments.
#### 2. Created by the Feeder:
Every STEP_ALIMENTAZIONE, the Feeder generates new atoms, assigning them an atomic number through the same pipe-based mechanism used by the Master.
#### 3. Created by another Atom
An Atom may split if it receives a split command from the Activator via the message queue. When the Atom reads a message with body 1, it attempts a fork().
- The split occurs only if the Atom’s atomic number is greater than or equal to the constant MIN_NUM_ATOMICO, defined in the configuration.
- If the condition is satisfied, the Atom splits: energy is released according to the defined policy, and the parent assigns half of its atomic number to the child.
- If the condition is not satisfied, the Atom becomes waste (scoria) and terminates.
All these operations update the Dump using mutual exclusion to safely access shared variables.

## Feeder
The Feeder process is created by the Master and is responsible for generating N_NUOVI_ATOMI every STEP_ALIMENTAZIONE nanoseconds. This timing is controlled using nanosleep().
Each time the Feeder creates a new Atom:
- It updates the counters tracking the number of atoms and the number of feeding operations performed within that second, using the associated semaphores.
- It assigns an atomic number to the Atom using the same mechanism employed by the Master.

  
## Activator
The Activator process, once created by the Master, attaches to shared memory to retrieve the IDs of the message queue and the Dump. It then waits for all processes to be fully initialized (synchronized via a semaphore reaching zero).
During the simulation, the Activator inserts 10 messages with msg_txt = 1 into the queue every STEP_ATTIVATORE. When read by an Atom, these messages trigger a split.
When the Activator receives a termination signal, its handler places a message with body 0 into the queue. When read by an Atom, this message instructs it to terminate.

## Termination
As explained earlier, termination is managed by the Master process. Before removing all shared resources, the Master sends SIGTERM signals to its processes, specifically the Activator and the Feeder.
Atoms receive their termination message from the Activator, which places a message with body 0 into the message queue. When an Atom reads this message, before terminating, it inserts another termination message into the queue for the next Atom to read.
