CFLAGS = -Wvla -Wextra -Werror -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

all: master atomo attivatore alimentazione

clean:
	rm -f *.o bin/master bin/atomo bin/attivatore bin/alimentazione *~

master: Master.c Makefile
	gcc $(CFLAGS) Master.c lib/GeneralUtils.c  -o bin/master

atomo: Atomo.c Makefile
	gcc $(CFLAGS)  Atomo.c lib/GeneralUtils.c -o bin/atomo

attivatore: Attivatore.c Makefile
	gcc $(CFLAGS) Attivatore.c lib/GeneralUtils.c -o bin/attivatore

alimentazione: Alimentazione.c Makefile
	gcc $(CFLAGS) Alimentazione.c lib/GeneralUtils.c -o bin/alimentazione

run:  all
	./bin/master
start:
	./bin/master