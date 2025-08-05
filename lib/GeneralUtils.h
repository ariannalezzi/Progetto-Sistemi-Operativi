#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "MacroUtil.h"

#include <signal.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>


#ifndef PROGETTOSO_LIBSEMAFORI_H
#define PROGETTOSO_LIBSEMAFORI_H
                /*************************************SEMAFORI***********************************************/

/**
 * questa funzione inizializza il semaforo per la sincronizzazione iniziale
 * della simulazione, il valore del semaforo é passato come parametro
 * @param sem_id
 * @param val
 */
void sem_init(int sem_id, int val, int n);


/**
 * questa funzione decrementa il valore del semaforo
 * gestisce l'errore
 * OSS: cercare di decrementare un semaforo oltre lo zero è
 * una operazione bloccante
 * @param sem_id
 */
void sem_down(int sem_id);

/**
 * questa funzione fa sì che il processo si metta in attesa
 * fino a ché il semaforo non arriva 0
 * @param sem_id
 */
void sem_wait_for_zero(int sem_id);
/**
 * funzione che svolge le operazioni di lettura da una pipe
 * e salva il contenuto nella variabile intera num
 * @param fd
 * @param num
 */
void lettura_pipe(int fd[2], int *num);
/**
 * scrive sulla pipe l'intero passatogli
 * @param fd
 * @param num
 */
void scrittura_pipe(int fd[2], int *num);
/**
 * la funzione fa un -1 sul semaforo n: n con semid
 * @param n
 * @param sem_id
 */
void P(int n, int sem_id);
/**
 * la funzione fa un +1 sul semaforo n: n con semid
 * @param n
 * @param sem_id
 */
void V(int n, int sem_id);

/**
 *
 * @param min
 * @param max
 * @return numero intro random tra min e max inclusi
 */
int random_btw(int min, int max);

/**
 * rimuove tutte le strutture di memoria condivisa
 * in uso nella simulazione
 */
void rimozioneStrutture();

/**
 * gestisce il segnale di terminazione , SIGTERM
 */
void gestore_segnale();
/**
 * la funzione calcola l'energia liberata dopo la scissione
 * @param n1 : numero atomico padre
 * @param n2 : numero atomico figlio
 * @return
 */
int energy(int n1, int n2);


/**Dump per riportare le statistiche ogni secondo*/
typedef struct Dump{
    int attivazione;
    int scissione;
    int energia_prodotta;
    int energia_consumata;
    int scoria_prodotta;
    int contatore_atomi;
    int alimetazioni;
}dump;


/**struttura atta a contenere i messaggi sulla coda di messaggi
 * 1 per scissione, 0 per morte in msg_text*/
typedef struct message {
    long msg_type;
    int msg_text;
} message;


#endif
