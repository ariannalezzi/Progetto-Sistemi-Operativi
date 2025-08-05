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
