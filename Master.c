#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"

int shmidInit, semid, msgid, shmid_dump;
int *shmidInit_ptr;
dump *dump_ptr;
int pid_attivatore;
int pid_alimentazione;


int main() {

    /**************************DICHIARAZIONI DI VARIABILI QUI !!*************************************************/
    int i;
    int N_ATOMI_INIT, ENERGY_DEMAND, MIN_N_ATOMICO, N_NUOVI_ATOMI, N_ATOM_MAX, STEP_ALIMENTAZIONE,
            ENERGY_EXPLODE_THRESHOLD, SIM_DURATION, STEP_ATTIVATORE;
    int current_second, continua;
    int config = 0;
    char *configuration;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);

    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];

    char buffer[10];
    char buffer2[10] = "dummy";

    char *args1[] = {"atomo", buffer, buffer2, NULL};
    char *args2[] = {"attivatore", buffer, NULL};
    char *args3[] = {"alimentazione", buffer, NULL};

    printf("Quale configurazione scegli? scrivi:  1 per timeout\n2 per explode\n3 per blackout\n4 per meltdown\n");
    fscanf(stdin, "%d", &config);
    switch (config) {
        case 1:
            printf("Hai scelto la configurazione che termina per timeout\n");
            configuration = "config_timeout.txt";
            break;
        case 2:
            printf("Hai scelto la configurazione che termina per explode\n");
            configuration = "config_explode.txt";
            break;
        case 3:
            printf("Hai scelto la configurazione che termina per blackout\n");
            configuration = "config_blackout.txt";
            break;
        case 4:
            printf("Hai scelto la configurazione che termina per meltdown\n");
            configuration = "config_meltdown.txt";
            break;

    }

    /*************************************** LETTURA PARAMETRI *****************************************************/
    FILE *file = fopen((const char *) configuration, "r");

    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
    if (fscanf(file,
               "N_ATOMI_INIT=%d\nENERGY_DEMAND=%d\nN_ATOM_MAX=%d\nMIN_N_ATOMICO=%d\nSTEP_ATTIVATORE=%d\nSTEP_ALIMENTAZIONE=%d\nN_NUOVI_ATOMI=%d\nSIM_DURATION=%d\nENERGY_EXPLODE_THRESHOLD=%d\n",
               &N_ATOMI_INIT, &ENERGY_DEMAND, &N_ATOM_MAX, &MIN_N_ATOMICO, &STEP_ATTIVATORE, &STEP_ALIMENTAZIONE,
               &N_NUOVI_ATOMI, &SIM_DURATION, &ENERGY_EXPLODE_THRESHOLD) != 9) {
        fprintf(stderr, "Errore nella lettura dei parametri di configurazione\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    /*************************************** FINE LETTURA PARAMETRI *****************************************************/

    printf("\nLA CONFIGURAZIONE:\n"
           "Numero atomi iniziali: %d\n"
           "Durata simulazione: %d\n"
           "Step Alimentazione : %d  nanosecondi\n"
           "Numero nuovi atomi creati da Alimentazione : %d\n"
           "Step Attivatore : %d secondi\n"
           "Numero di attivazioni : %d \n"
           "Soglia di esplosione: %d \n\n", N_ATOMI_INIT, SIM_DURATION, STEP_ALIMENTAZIONE, N_NUOVI_ATOMI,
           STEP_ATTIVATORE, 10, ENERGY_EXPLODE_THRESHOLD);
    sleep(1);
    /*************************************** CREAZIONE STRUTTURE *****************************************************/


    /*Creazione memoria condivisa per passaggio parametri + ids*/
    shmidInit = shmget(IPC_PRIVATE, sizeof(int) * (10) + 1, IPC_CREAT | 0666);
    TEST_ERROR

    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    TEST_ERROR

    /* Creazione MEMORIA CONDIVISA per le statistiche ogni secondo */
    shmid_dump = shmget(IPC_PRIVATE, sizeof(struct Dump), IPC_CREAT | 0666);
    TEST_ERROR

    dump_ptr = (struct Dump *) shmat(shmid_dump, NULL, 0);
    TEST_ERROR

    /* creazione 8 semafori
     * 0)sincronizzazione processi iniziali
     * 1)attivazione
     * 2)scissione
     * 3)energia prodotta
     * 4)energia consumata
     * 5)scorie prodotta
     * 6)contatore Atomi
     * 7)alimentazioni*/

    semid = semget(IPC_PRIVATE, 8, IPC_CREAT | 0666);
    TEST_ERROR

    /* Creazione message queue */
    msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    TEST_ERROR


    /*************************************** FINE CREAZIONE STRUTTURE *****************************************************/

    /*************************************** INIZIALIZZAZIONI *************************************************************/
    /* Riempimento memoria condivisa */
    shmidInit_ptr[0] = ENERGY_DEMAND;
    shmidInit_ptr[1] = MIN_N_ATOMICO;
    shmidInit_ptr[2] = ENERGY_EXPLODE_THRESHOLD;
    shmidInit_ptr[3] = STEP_ATTIVATORE;
    shmidInit_ptr[4] = SIM_DURATION;
    shmidInit_ptr[5] = STEP_ALIMENTAZIONE;
    shmidInit_ptr[6] = N_NUOVI_ATOMI;
    shmidInit_ptr[7] = N_ATOM_MAX;
    shmidInit_ptr[8] = shmid_dump;
    shmidInit_ptr[9] = semid;
    shmidInit_ptr[10] = msgid;


    if (sprintf((char *) args1[1], "%d", shmidInit) < 0) {
        printf("ERRORE sprintf shmidInit in master");
    }

    /**seed initialize**/
    srand(time(NULL));


    /*Inizializzo semaforo per dump*/
    sem_init(semid, N_ATOMI_INIT + 1 + 1 + 1, 0);

    /*inizializzo il semaforo*/
    for (int j = 1; j < 8; j++) {
        sem_init(semid, 1, j);
    }

    /**init array pids**/
    P(6, semid);
    dump_ptr->contatore_atomi = 0;
    V(6, semid);


    /*************************************** FINE INIZIALIZZAZIONI *************************************************************/

    for (i = 0; i < N_ATOMI_INIT; i++) {
        if (pipe(file_pipe) == -1) {
            perror("fail in pipe atomo in master");
            exit(EXIT_FAILURE);
        }
        switch (fork()) {
            case -1:
                perror("fork fail for atomo in master");
                exit(EXIT_FAILURE);
            case 0:
                sem_down(semid);
                printf("Atomo(initial) %d waiting for 0 before fire execve \n", getpid());

                P(6, semid);
                dump_ptr->contatore_atomi++;
                V(6, semid);

                /*ricezione numero atomico*/
                close(file_pipe[1]);
                read(file_pipe[0], &num, sizeof(int));
                close(file_pipe[0]);
                TEST_ERROR
                sprintf((char *) args1[2], "%d", num);


                if (execve("bin/atomo", args1, NULL) == -1) {
                    perror("execve fail atomo in master");
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


    pid_attivatore = fork();
    switch (pid_attivatore) {
        case -1:
            perror("error fork attivatore in master");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);

            printf("Attivatore con pid %d \n", getpid());

            if (execve("bin/attivatore", args2, NULL) == -1) {
                perror("execve attivatore fail in master");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }


    pid_alimentazione = fork();
    switch (pid_alimentazione) {
        case -1:
            perror("error fork alimentazione in master");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);

            printf("Alimentazione con pid %d", getpid());

            if (execve("bin/alimentazione", args3, NULL) == -1) {
                perror("execve alimentazione fail in master");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    sleep(1);

    sem_down(semid);

    printf("Master  %d waiting for 0...\n", getpid());
    sem_wait_for_zero(semid);

    printf("||| ---------------------------------------------- INIZIO SIMULAZIONE --------------------------------------------------------- |||\n");

    int attivazioni = 0, scissioni = 0, en_prod = 0, en_cons = 0, scorie = 0, alim = 0;

    current_second = 0;
    continua = 1;

    while (current_second <= SIM_DURATION && continua) {
        for (int t = 1; t < 8; t++)
            P(t, semid);

        /*************************** STATISTICHE (al sec) **********************************************************/
        printf("\n\n\n");
        printf("|------------------------------------------------------------------------------------------------------------------------------------------|\n");
        printf("                                                 SECONDO %d\n", current_second);
        printf("|------------------------------------------------------------------------------------------------------------------------------------------|\n");
        printf("|-----------------|---------------|----------------------|-------------------------|------------|||---------------|------------------------|\n");
        printf("|   ATTIVAZIONI   |   SCISSIONI   |   ENERGIA PRODOTTA   |    ENERGIA CONSUMATA    |   SCORIE   ||| ALIMENTAZIONI |   ENERGIA DISPONIBILE  |\n");
        printf("|-----------------|---------------|----------------------|-------------------------|------------|||---------------|------------------------|\n");
        printf("|     %3d         |     %3d       |         %3d          |           %3d           |    %3d     |||     %3d       |         %3d            |\n",
               dump_ptr->attivazione - attivazioni,
               dump_ptr->scissione - scissioni,
               dump_ptr->energia_prodotta - en_prod,
               dump_ptr->energia_consumata - en_cons,
               dump_ptr->scoria_prodotta - scorie,
               dump_ptr->alimetazioni - alim,
               dump_ptr->energia_prodotta - dump_ptr->energia_consumata);
        printf("\n");
        printf("|-----------------|---------------|----------------------|-------------------------|------------|||--------------|--------------|\n");
        printf("| ATTIVAZIONI TOT | SCISSIONI TOT | ENERGIA PRODOTTA TOT |  ENERGIA CONSUMATA TOT  | SCORIE TOT ||| ATOMI ATTIVI | ATOMI TOTALI |\n");
        printf("|-----------------|---------------|----------------------|-------------------------|------------|||--------------|--------------|\n");
        printf("|     %3d         |     %3d       |         %3d          |           %3d           |    %3d     |||    %3d       |      %3d     |\n",
               dump_ptr->attivazione,
               dump_ptr->scissione,
               dump_ptr->energia_prodotta,
               dump_ptr->energia_consumata,
               dump_ptr->scoria_prodotta,
               dump_ptr->contatore_atomi - dump_ptr->scoria_prodotta,
               dump_ptr->contatore_atomi);
        printf("|-----------------|---------------|----------------------|-------------------------|------------|||--------------|--------------|\n");
        printf("\n");
        /**aggiornamento valori temporanei **/
        attivazioni = dump_ptr->attivazione;
        scissioni = dump_ptr->scissione;
        en_prod = dump_ptr->energia_prodotta;
        en_cons = dump_ptr->energia_consumata;
        scorie = dump_ptr->scoria_prodotta;
        alim = dump_ptr->alimetazioni;
        /**controllo per esplosione e per blackout**/
        if (dump_ptr->energia_prodotta - dump_ptr->energia_consumata > ENERGY_EXPLODE_THRESHOLD ||
            dump_ptr->energia_prodotta - dump_ptr->energia_consumata < 0) {
            continua = 0;
        }

        for (int m = 1; m < 8; m++)
            V(m, semid);
        if (continua > 0) {
            P(3, semid);
            printf("|-------------------------------------------------------CONSUMO ENERGIA %3d---------------------------------------------------|\n",
                   ENERGY_DEMAND);
            printf("\n\n\n");
            dump_ptr->energia_consumata += ENERGY_DEMAND;
            V(3, semid);
        }

        current_second++;
        sleep(1);
    }
    printf("\n\nINIZIA LA TERMINAZIONE DEI PROCESSI \n\n\n");


    if (kill(pid_attivatore, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_attivatore);
    } else {
        perror("Errore durante l'invio del segnale terminazione all'attivatore");
        return 1;
    }
    if (kill(pid_alimentazione, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_alimentazione);
    } else {
        perror("Errore durante l'invio del segnale");
        return 1;
    }


    /**************/
    printf("Terminazione atomi .... \n");
    sleep(5);

    if (current_second > SIM_DURATION) {
        printf("La simulazione e' terminata perche' sono trascorsi %d secondi quindi per TIMEOUT\n", SIM_DURATION);
    } else if ((en_prod - en_cons) >= ENERGY_EXPLODE_THRESHOLD) {
        printf("La simulazione e' terminata per EXPLODE, l'energia liberata e' maggiore di %d\n",
               ENERGY_EXPLODE_THRESHOLD);
        printf("Energia liberata: %d\n", (en_prod - en_cons));
    } else if (en_prod - en_cons < 0) {
        printf("dump_ptr->energia_prodotta - dump_ptr->energia_consumata =  %d\n",
               -en_cons + en_prod);
        printf("Caso BLACKOUT \n\n\n");
    }

    /*************************** STATISTICHE (totali) **********************************************************/



    /*------------------Rimozione dei strutture e della memoria condivisa---------------------------- */
    rimozioneStrutture();

    printf("\n||| ------------------------------------------------FINE SIMULAZIONE--------------------------------------------------------- |||\n");

    exit(EXIT_SUCCESS);

}


void rimozioneStrutture() {



    /* detach memoria condivisa dal processo per dump */
    if (shmdt(dump_ptr) == -1) {
        perror("errore rimozione dump_ptr");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo per Init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("errore rimozione SHMINIT_PTR");
        exit(EXIT_FAILURE);
    }

    /* rimozione memoria condivisa per dump */
    if (shmctl(shmid_dump, IPC_RMID, NULL) == -1) {
        perror("errore rimozione shmid_dump");
        exit(EXIT_FAILURE);
    }

    for (int (n) = 0; (n) < 8; ++(n)) {
        semctl(semid, n, IPC_RMID);
    }
    if (shmctl(shmidInit, IPC_RMID, NULL) == -1){
        perror("errore rimozione shmidInit");
        exit(EXIT_FAILURE);
    }
    if (msgctl(msgid, IPC_RMID, NULL) == -1){
        perror("errore rimozione msgid");
        exit(EXIT_FAILURE);
    }
}


void gestore_segnale() {


    printf("CASO ERRORE FORK: MELTDOWN\n");

    if (kill(pid_attivatore, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_attivatore);
    } else {
        perror("Errore durante l'invio del segnale terminazione attivatore in master per meltdown");
    }

    sleep(5);

    rimozioneStrutture();
    exit(EXIT_SUCCESS);
}

