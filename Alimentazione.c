#include "lib/GeneralUtils.h"

struct Dump *dump_ptr;


int main(int argc, char *argv[]) {
    int i;
    int STEP_ALIMENTAZIONE, N_NUOVI_ATOMI, N_ATOM_MAX;
    char buffer[10];
    char buffer1[10];
    int numero_atomico;
    int num;
    int file_pipe[2];
    int shmidInit, semid, dump_id;
    int *shmidInit_ptr;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);
    struct timespec structtimespec;
    int flag = 1;


    if (argc != 2) {
        fprintf(stderr, "Error, wrong number of argument for Alimentazione\n");
        exit(EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    sprintf(buffer, "%d", shmidInit);

    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        TEST_ERROR
    }

    STEP_ALIMENTAZIONE = shmidInit_ptr[5];
    N_NUOVI_ATOMI = shmidInit_ptr[6];
    N_ATOM_MAX = shmidInit_ptr[7];
    semid = shmidInit_ptr[9];
    dump_id = shmidInit_ptr[8];


    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        TEST_ERROR
    }
    /*Collega la memoria condivisa dump al puntatore*/
    dump_ptr = (struct Dump *) shmat(dump_id, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        TEST_ERROR
    }

    char *args1[] = {"atomo", buffer, buffer1, NULL};


    printf("Alimentazione waiting for zero\n");
    sem_wait_for_zero(semid);
    structtimespec.tv_nsec = STEP_ALIMENTAZIONE;
    structtimespec.tv_sec = 0;
    nanosleep(&structtimespec, NULL);
    while (flag) {
        for (i = 0; i < N_NUOVI_ATOMI; i++) {
            if (pipe(file_pipe) == -1) {
                perror("fail in pipe alimentazione");
                exit(EXIT_FAILURE);
            }

            switch (fork()) {
                case -1:
                    flag = 0;
                    TEST_ERROR
                    rimozioneStrutture();
                    kill(getppid(), SIGTERM);
                    exit(EXIT_FAILURE);
                case 0:

                    P(6, semid);
                    dump_ptr->contatore_atomi++;
                    V(6, semid);

                    P(7, semid);
                    dump_ptr->alimetazioni++;
                    V(7, semid);

                    close(file_pipe[1]);
                    read(file_pipe[0], &num, sizeof(int));
                    close(file_pipe[0]);
                    sprintf(args1[2], "%d", num);

                    if (errno == EAGAIN) {
                        exit(EXIT_FAILURE);
                    }
                    if (execve("bin/atomo", args1, NULL) == -1) {
                        perror("execve fail in alimentazione");
                        exit(EXIT_FAILURE);
                    }


                    exit(EXIT_FAILURE);
                default:
                    close(file_pipe[0]);
                    numero_atomico = random_btw(1, N_ATOM_MAX);
                    write(file_pipe[1], &numero_atomico, sizeof(int));
                    break;
            }
            close(file_pipe[1]);
        }
        nanosleep(&structtimespec, NULL);
    }
}

void rimozioneStrutture() {
    /* detach memoria condivisa dal processo*/
    if (shmdt(dump_ptr) == -1) {
        TEST_ERROR
    }
}

void gestore_segnale() {

    rimozioneStrutture();
    TEST_ERROR
    exit(EXIT_SUCCESS);
}
