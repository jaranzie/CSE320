#include <stdlib.h>

#include "debug.h"
#include "polya.h"
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
/*
 * master
 * (See polya.h for specification.)
 */

typedef struct worker_pipe {
    FILE* write;
    FILE* read;
    pid_t child_pid;
    volatile char status;
} worker_pipe;

static int numWorkers;
static worker_pipe* workers_pipes;

void SIGCHILD_HANDLER(int sig) {
    sigset_t mask_all, prev_all;
    pid_t pid;

    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    int old_errno = errno;
    int child_status = 0;
    while ((pid = waitpid(-1, &child_status, (WNOHANG | WUNTRACED | WCONTINUED))) > 0)  { /* Reap child */
        for(int i = 0; i < numWorkers; i++) {
            if(workers_pipes[i].child_pid == pid) {
                if(workers_pipes[i].status == WORKER_STARTED && WIFSTOPPED(child_status)) {
                    workers_pipes[i].status = WORKER_IDLE;
                    sf_change_state(pid, WORKER_STARTED, WORKER_IDLE);
                } else if (workers_pipes[i].status == WORKER_CONTINUED && WIFCONTINUED(child_status)) {
                    workers_pipes[i].status = WORKER_RUNNING;
                    sf_change_state(pid, WORKER_CONTINUED, WORKER_RUNNING);
                } else if (workers_pipes[i].status == WORKER_RUNNING && WIFSTOPPED(child_status)) {
                    workers_pipes[i].status = WORKER_STOPPED;
                    sf_change_state(pid, WORKER_RUNNING, WORKER_STOPPED);
                } else if (workers_pipes[i].status == WORKER_CONTINUED && WIFSTOPPED(child_status)) {
                    workers_pipes[i].status = WORKER_RUNNING;
                    sf_change_state(pid, WORKER_CONTINUED, WORKER_RUNNING);
                    workers_pipes[i].status = WORKER_STOPPED;
                    sf_change_state(pid, WORKER_RUNNING, WORKER_STOPPED);
                } else if (WIFEXITED(child_status)) {
                    char old_state = workers_pipes[i].status;
                    if(WEXITSTATUS(child_status) == EXIT_SUCCESS) {
                        workers_pipes[i].status = WORKER_EXITED;
                        sf_change_state(pid, old_state, WORKER_EXITED);
                    } else {
                        workers_pipes[i].status = WORKER_ABORTED;
                        sf_change_state(pid, old_state, WORKER_ABORTED);
                    }
                } else if(!WIFSTOPPED(child_status) && !WIFCONTINUED(child_status)) {
                    workers_pipes[i].status = WORKER_ABORTED;
                }
                break;
            }
        }

    }
    if (errno != ECHILD) {
        errno = old_errno;
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
}

int master(int workers) {
    sf_start();
    signal(SIGCHLD, SIGCHILD_HANDLER);
    workers_pipes = malloc(sizeof(worker_pipe)*workers);
    numWorkers = workers;
    sigset_t mask_all, mask_child, prev_one;
    sigfillset(&mask_all);
    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_one);
    for(int i = 0; i < workers;i++) {
        int to_worker[2];
        int from_worker[2];
        pipe(to_worker);
        pipe(from_worker);
        int fork_res = fork();
        if (fork_res == -1) {
            /* Fork Error */
            close(from_worker[0]);
            close(to_worker[1]);
            close(from_worker[1]);
            close(to_worker[0]);
            i--;
            continue;
        } else if (fork_res == 0) {
            /* CHILD */
            close(from_worker[0]);
            close(to_worker[1]);
            dup2(from_worker[1], STDOUT_FILENO);
            dup2(to_worker[0], STDIN_FILENO);
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            execlp("./bin/polya_worker", "polya_worker", NULL);
        } else {
            /* MASTER  */
            worker_pipe newPipe;
            close(from_worker[1]);
            close(to_worker[0]);
            newPipe.read = fdopen(from_worker[0], "r");
            newPipe.write = fdopen(to_worker[1], "w");
            newPipe.child_pid = fork_res;
            newPipe.status = WORKER_STARTED;
            sf_change_state(fork_res, 0, WORKER_STARTED);
            workers_pipes[i] = newPipe;
        }
    }
    debug("[%d:MASTER] Finished making children", getpid());
    sigprocmask(SIG_SETMASK, &prev_one, NULL);

    for(int i = 0; i < workers; i++) { /* VERTIFY ALL WORKERS IDLE */
        if(workers_pipes[i].status != WORKER_IDLE && workers_pipes[i].status != WORKER_ABORTED) {
            sigsuspend(&prev_one);
        }

    }
    while (1) {
        int possible_n_var = workers;
        struct problem* current_problem = get_problem_variant(possible_n_var, 0);
        while (current_problem == NULL) {
            possible_n_var -= 1;
            if(possible_n_var == 0) {
                break;
            }
            current_problem = get_problem_variant(possible_n_var, 0);
        }
        if(possible_n_var == 0) { /* No Problems Left */
            while(1) {
                int num_idle = 0;
                int num_aborted = 0;
                for(int i = 0; i < workers; i++) {
                    if (workers_pipes[i].status == WORKER_IDLE) {
                        num_idle += 1;
                    }
                    if(workers_pipes[i].status == WORKER_ABORTED) {
                        num_aborted++;
                    }
                }
                if ((num_idle + num_aborted) == workers) {
                    break;
                }
            }
            sigprocmask(SIG_BLOCK, &mask_child, &prev_one);
            for(int i = 0; i < workers; i++) { /* TERMINATE WORKERS */
                kill(workers_pipes[i].child_pid, SIGTERM);
                kill(workers_pipes[i].child_pid, SIGCONT);
            }
            sigsuspend(&prev_one);
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            break;
        }
        sigprocmask(SIG_BLOCK, &mask_all, &prev_one);

        int num_aborted = 0;
        for(int i = 0; i < workers; i++) {
            if(workers_pipes[i].status == WORKER_ABORTED) {
                num_aborted++;
            }
        }
        if(num_aborted == workers) {
            break;
        }

        for(int i = 0; i < possible_n_var; i++) {
            if (workers_pipes[i].status == WORKER_IDLE) {
                current_problem = get_problem_variant(possible_n_var, i);
                sf_send_problem(workers_pipes[i].child_pid, current_problem);
                fwrite(current_problem, 1, current_problem->size, workers_pipes[i].write);
                fflush(workers_pipes[i].write);
                workers_pipes[i].status = WORKER_CONTINUED;
                sf_change_state(workers_pipes[i].child_pid, WORKER_IDLE, WORKER_CONTINUED);
                kill(workers_pipes[i].child_pid, SIGCONT);
            }
        }
        sigsuspend(&prev_one);
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        struct result res;
        struct result* final_res = NULL;
        while (1) {
            num_aborted = 0;
            for(int i = 0; i < possible_n_var; i++) {
                if(workers_pipes[i].status == WORKER_ABORTED) {
                    num_aborted++;
                }
                if (workers_pipes[i].status == WORKER_STOPPED) {
                    fread(&res,sizeof(struct result), 1, workers_pipes[i].read);
                    workers_pipes[i].status = WORKER_IDLE;
                    sf_change_state(workers_pipes[i].child_pid, WORKER_STOPPED, WORKER_IDLE);
                    final_res = malloc(res.size);
                    *final_res = res;
                    fread(&final_res->data,1, res.size - sizeof(struct result), workers_pipes[i].read);
                    sf_recv_result(workers_pipes[i].child_pid, final_res);
                    if(final_res->failed) {
                        free(final_res);
                        final_res = NULL;
                        continue;
                    }
                    break;
                }
            }
            if(final_res != NULL) {
                break;
            } else if (num_aborted == workers) {
                break;
            }
        }
        if (num_aborted == workers) {
            break;
        }
        if (post_result(final_res,current_problem)) {
            /* Problem not solved correctly */
            debug("[%d:Master] Posted result does not solve the problem", getpid());
        }
        free(final_res);
        /* SIGUP all processes still running */
        for(int i = 0; i < possible_n_var; i++) {
            if (workers_pipes[i].status == WORKER_RUNNING) {
                kill(workers_pipes[i].child_pid, SIGHUP);
                sf_cancel(workers_pipes[i].child_pid);
                sigsuspend(&prev_one);
            }
        }
        for(int i = 0; i < possible_n_var; i++) { /* Clearing pipes */
            if (workers_pipes[i].status == WORKER_STOPPED) {
                struct result extra;
                fread(&extra,sizeof(struct result), 1, workers_pipes[i].read);
                sf_recv_result(workers_pipes[i].child_pid, &extra);
                workers_pipes[i].status = WORKER_IDLE;
                fseek(workers_pipes[i].read, (long)extra.size - (long)sizeof(struct result), SEEK_CUR);
                sf_change_state(workers_pipes[i].child_pid, WORKER_STOPPED, WORKER_IDLE);
            } else if (workers_pipes[i].status != WORKER_IDLE) {
                continue;
            }
        }
    }
    for(int i = 0; i < workers; i++) {
        fclose(workers_pipes[i].read);
        fclose(workers_pipes[i].write);
    }
    while(1) {
        int num_aborts = 0;
        for(int i = 0; i < workers; i++) {
            if (!(workers_pipes[i].status == WORKER_EXITED || workers_pipes[i].status == WORKER_ABORTED)) {
                break;
            }
            if (workers_pipes[i].status == WORKER_ABORTED) {
                num_aborts++;
            }
            if (i == (workers - 1)) {
                sf_end();
                free((void*)workers_pipes);
                if(num_aborts == 0) {
                    exit(EXIT_SUCCESS);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}
