#include "lib/GeneralUtils.h"


dump *dump_ptr;
message myMessage;
int msgid;

int main(int argc, char *argv[]) {

    int shmidReport, i = 0;
    int shmidInit, STEP_ATTIVATORE;
    int *shmidInit_ptr;
    int semid;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);

    if (argc != 2) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        TEST_ERROR
    }

    STEP_ATTIVATORE = shmidInit_ptr[3];
    semid = shmidInit_ptr[9];
    shmidReport = shmidInit_ptr[8];
    msgid = shmidInit_ptr[10];



    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt shmidInit_ptr fail in attivatore");
        exit(EXIT_FAILURE);
    }



    /*Collega la memoria condivisa Dump al puntatore*/
    dump_ptr = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) dump_ptr == (void *) (-1)) {
        perror("shmat puntatore dump fail in attivatore");
        exit(EXIT_FAILURE);
    }


    printf("Attivatore waiting for zero... \n");

    sem_wait_for_zero(semid);

    while (1) {
        i = 0;

        while (i < 10) {
            myMessage.msg_type = 1;
            myMessage.msg_text = 1;
            msgsnd(msgid, &myMessage, sizeof(int), 0);
            P(1, semid);
            dump_ptr->attivazione = dump_ptr->attivazione + 1;
            V(1, semid);
            i++;
        }
        sleep(STEP_ATTIVATORE);
    }


}


void gestore_segnale() {

    myMessage.msg_type = 1;
    myMessage.msg_text = 0;
    msgsnd(msgid, &myMessage, sizeof(int), 0);

    if (shmdt(dump_ptr) == -1) {
        perror("shmdt puntatore dump fail in attivatore");
        exit(EXIT_FAILURE);
    }
    printf("Detach in attivatore avvenuto\n");

    exit(EXIT_SUCCESS);
}
