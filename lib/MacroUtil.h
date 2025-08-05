#ifndef PROGETTOSO_MACROUTIL_H
#define PROGETTOSO_MACROUTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO, \
        "%s:%d: PID=%5d: Error %d (%s)\n",\
        __FILE__,\
        __LINE__,\
        getpid(),\
        errno,\
        strerror(errno));}

#endif
