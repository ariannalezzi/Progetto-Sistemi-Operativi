#include <stdlib.h>
#include "GeneralUtils.h"


void sem_init(int sem_id, int val, int n) {
    if (semctl(sem_id, n, SETVAL, val) == -1) {
        TEST_ERROR
        exit(EXIT_FAILURE);
    }
}

void sem_down(int sem_id) {
    struct sembuf sem_op = {0, -1, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void sem_wait_for_zero(int sem_id) {
    struct sembuf sem_op = {0, 0, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

void P(int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = -1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}

void V(int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = 1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}

void lettura_pipe(int fd[2], int *num) {
    close(fd[1]);
    read(fd[0], num, sizeof(int));
    close(fd[0]);
}

void scrittura_pipe(int fd[2], int *num) {
    close(fd[0]);
    write(fd[1], num, sizeof(int));

}


int random_btw(int min, int max) {
    return rand() % (max - min + 1) + min;
}


int max(int n1, int n2) {
    if (n1 > n2)
        return n1;
    else return n2;
}

int energy(int n1, int n2) {
    return n1 * n2 - max(n1, n2);
}


