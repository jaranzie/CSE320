#include <stdlib.h>

#include "debug.h"
#include "polya.h"
#include <unistd.h>

/*
 * worker
 * (See polya.h for specification.)
 */

volatile static int sig_flags = 0;
static FILE* to_master;
static FILE* from_master;
static pid_t master_pid;
static volatile sig_atomic_t sigterm_flag = 0;
static volatile sig_atomic_t sighup_flag = 0;

void SIGHUP_HANDLER(int sig) {
    debug("[%d:SIGHUP] SIGHUP flag", getpid());
    sighup_flag = 1;
}

void SIGTERM_HANDLER(int sig) {
    debug("[%d:SIGTERM HANDLER] Sigterm flag", getpid());
    _exit(EXIT_SUCCESS);
    sigterm_flag = 1;
}
// write to stdout
// read from stdin
int worker(void) {
//    short success;
    signal(SIGHUP, SIGHUP_HANDLER);
    signal(SIGTERM, SIGTERM_HANDLER);
    to_master = fdopen(STDOUT_FILENO, "w");
    from_master = fdopen(STDIN_FILENO, "r");
    master_pid = getppid();
    struct problem* current_problem;
    while (1) {
        /*success = 0;*/
        kill(getpid(), SIGSTOP);
        if (sigterm_flag != 0) { /* CHeck sigterm */
            fclose(to_master);
            fclose(from_master);
            exit(EXIT_SUCCESS);
        }
        /*raise(SIGSTOP);*/
        struct problem p;
        if (fread(&p, sizeof(struct problem), 1, from_master) != 1) { /* Read problem */
            debug("[%d:child] read failed", getpid());
        }
        size_t problem_size = p.size - sizeof(struct problem); /* Read Problem data */
        current_problem = malloc(p.size);
        *current_problem = p;
        if (fread(&current_problem->data, 1, problem_size, from_master) != problem_size) {
            /* Problem reading problem data */
        }
        while (1) {
            if (sigterm_flag != 0) { /* Check sigterm */
                fclose(to_master);
                fclose(from_master);
                free(current_problem);
                exit(EXIT_SUCCESS);
            } else {
                sighup_flag = 0;
                struct result* res = solvers[current_problem->type].solve(current_problem,&sighup_flag);
                if (res == NULL) {
                    struct result failed_res = {.size = sizeof(struct result), .failed = 1};
                    if (fwrite(&failed_res, 1, failed_res.size, to_master) != failed_res.size) {
                        debug("[%d:child] write failed", getpid());
                    }
                    fflush(to_master);
                    free(current_problem);
                    break;
                } else {
                    if (fwrite(res, 1, res->size, to_master) != res->size) {
                        debug("[%d:child] write failed", getpid());
                    }
                    fflush(to_master);
                    free(current_problem);
                    free(res);
                    break;
                }

            }
        }
    }
}
