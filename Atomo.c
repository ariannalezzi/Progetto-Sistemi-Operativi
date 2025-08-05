#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"

dump *dump_ptr;


int main(int argc, char *argv[]) {
    int MIN_N_ATOMICO;
    int msgid;
    int fd[2];
    struct message myMessage;
    int num_bytes;
    int numero_atomico, num_figlo_i, num_figlio_r, shmidInit;
    int *shmidInit_ptr;
    int semid;
    int dump_id;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);


    char buffer[10];
    char buffer1[10];
    char *args1[] = {"atomo", buffer, buffer1, NULL};
    if (argc != 3) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    numero_atomico = atoi(argv[2]);

    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        TEST_ERROR
    }
    sprintf(buffer, "%d", shmidInit);
    MIN_N_ATOMICO = shmidInit_ptr[1];
    msgid = shmidInit_ptr[10];
    semid = shmidInit_ptr[9];
    dump_id = shmidInit_ptr[8];


    dump_ptr = (struct Dump *) shmat(dump_id, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        TEST_ERROR
    }
    /* detach memoria condivisa per array pid*/
    if (shmdt(shmidInit_ptr) == -1) {
        TEST_ERROR
    }
    myMessage.msg_text = 1;

    sem_wait_for_zero(semid);

    while (1) {
        num_bytes = 0;
        num_bytes = msgrcv(msgid, &myMessage, sizeof(int), 1, 0);
        if (num_bytes < 0) {
            if (errno == ENOMSG) {
                printf("CHILD %d: nessun messaggio di quel tipo", getpid());
                sleep(2);
                continue;
            } else {
                printf("Errore n %d: %s", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (num_bytes > 0) {
            if (myMessage.msg_text == 0) {
                myMessage.msg_text = 0;
                myMessage.msg_type = 1;
                msgsnd(msgid, &myMessage, sizeof(int), 0);
                gestore_segnale();
            }
            if (numero_atomico <= MIN_N_ATOMICO) {

                P(5, semid);
                dump_ptr->scoria_prodotta++;
                V(5, semid);

                gestore_segnale();
            }

            if (pipe(fd) == -1) {
                TEST_ERROR
                exit(EXIT_FAILURE);
            }
            switch (fork()) {
                case -1:
                    TEST_ERROR
                    exit(EXIT_FAILURE);
                case 0:
                    lettura_pipe(fd, &num_figlio_r);

                    P(2, semid);
                    dump_ptr->scissione++;
                    V(2, semid);

                    P(6, semid);
                    dump_ptr->contatore_atomi++;
                    V(6, semid);

                    P(3, semid);
                    dump_ptr->energia_prodotta += (energy(numero_atomico - num_figlio_r, num_figlio_r));
                    V(3, semid);

                    sprintf(args1[2], "%d", num_figlio_r);
                    if (execve("bin/atomo", args1, NULL) == -1) {
                        perror("execve fail in atomo");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_FAILURE);
                default:
                    num_figlo_i = numero_atomico - numero_atomico / 2;
                    numero_atomico = numero_atomico - num_figlo_i;
                    scrittura_pipe(fd, &num_figlo_i);
                    break;
            }
            close(fd[1]);
        }

    }

}


void rimozioneStrutture() {
    if (shmdt(dump_ptr) == -1) {
        perror("shmdt dump_ptr in atomo");
        exit(EXIT_FAILURE);
    }
}

void gestore_segnale() {
    rimozioneStrutture();
    TEST_ERROR
    exit(EXIT_SUCCESS);
}



