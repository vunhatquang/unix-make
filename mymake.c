/* File: mymake.c
 * This file contains the main function, which will control mymake program
*/

#define _POSIX_SOURCE
#include "mymake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

// with this option, the default makefile is replaced with file mf. without this option, 
// the program will search for the default mymake1.mk, mymake2.mk, mymake3.mk
char *mf = NULL;
// if p_flag is on, build a database from make file, output and exit
int p_flag;
// if k flag is on, mymake should continue execution even when some command fail
int k_flag;
// if d flag is on, mymake should print debugging information while executing
int d_flag;
// if i flag is on, mymake should block the SIGINT signal so that ctrl-c would not have 
// effect on the program, without it, mymake should clean up (kill) all of the children 
// then exit when ctrl-c is typed
int i_flag;
// if this option is set, mymake should run roughly num seconds. If it reach num seconds, it 
// should gracefully self-destruct (clean up all of its children and then exit)
int t_num;
// if target is not presented in the command, the default target is the first target in the 
// target rule in the makefile
char *target = NULL;

static void sig_int(int signo) {
    if (i_flag != 1) {
        if (kill(0, SIGTERM) != 0)
            kill(0, SIGKILL);
        exit(0);
    }
    printf("i flag is one\n");
}

void sig_alrm( int sig ) {
    if (t_num != -1) {
        if (kill(0, SIGTERM) != 0)
            kill(0, SIGKILL);
        exit(0);
    }
}

void debug() {
    printf("target: %s\n", target);
    printf("p: %d\n", p_flag);
    printf("k: %d\n", k_flag);
    printf("d: %d\n", d_flag);
    printf("i: %d\n", i_flag);
    printf("t num: %d\n", t_num);
    printf("mf: %s\n", mf);
}

int main(int argc, char *argv[]) {
    // set flag and params to default value
    p_flag = 0;
    k_flag = 0;
    d_flag = 0;
    i_flag = 0;
    t_num = -1;
    FILE *mFile;
    int i;
    struct stat statbuf;
    char *my_path;

    sigset_t newmask, oldmask;
    struct sigaction abc, act;
    abc.sa_handler = sig_int;
    sigemptyset(&abc.sa_mask);
    abc.sa_flags = 0;
    sigaction(SIGINT, &abc, NULL);

    


    // dispatch parameters
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
                mf = argv[i + 1];
                i++;
        } else if (strcmp(argv[i], "-p") == 0) {
                p_flag = 1;
        } else if (strcmp(argv[i], "-k") == 0) {
                k_flag = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
                d_flag = 1;
        } else if (strcmp(argv[i], "-i") == 0) {
                i_flag = 1;
        } else if (strcmp(argv[i], "-t") == 0) {
                t_num = atoi(argv[i + 1]);
                i++;
        } else {
                target = argv[i];
        }
    }

    if (i_flag == 1) {
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGINT);
        if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
            perror("sigprocmask");
    }

    if (t_num != -1) {
        act.sa_handler = sig_alrm;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGALRM, &act, 0);
        alarm(t_num);
    }

    // check on mf and set default values if mf is NULL
    if (mf == NULL) {
        if (stat("mymake1.mk", &statbuf) >= 0) {
            mf = "mymake1.mk";
        } else if (stat("mymake2.mk", &statbuf) >= 0) {
            mf = "mymake2.mk";
        } else if (stat("mymake3.mk", &statbuf) >= 0) {
            mf = "mymake3.mk";
        } else {
            perror(mf);
            return 1;
        }
    }

    // TODO: line below is for testing purpose only
    setenv("MYPATH", "/home/grads/nvu/.bin:/home/grads/nvu/.scripts:/usr/local/bin:/opt/sfw/bin:/usr/sfw/bin:/bin:/usr/bin:/usr/ccs/bin:/usr/ucb:.", 1);

    my_path = getenv("MYPATH");
    processPath(my_path);

    
    mFile = fopen(mf,"r");
    if (mFile == NULL) {
        perror(mf);
        return 1;
    }
    if (!readFile(mFile)) {
        fclose(mFile);
        freeGraph();
        return 1;
    }
    fclose(mFile);

    // set target to first target in struct if target is NULL
    if (target == NULL) {
        target = head->tars->str;
    }


    // if p option is use
    if (p_flag == 1) {
        printStruct();
    } else {
        // TODO traverse the graph and execute the command
        travGraph(target);
    }

    freeGraph();
    if (i_flag == 1) {
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) 
        perror("sigprocmask");
    }
    return 0;
}