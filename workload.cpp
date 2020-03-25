/* Synthetic workload demonstrating multithreading and multiprocessing. */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

static pid_t _private_gettid(void)
{
    return (pid_t)syscall(SYS_gettid);
}
#define gettid _private_gettid


struct WorkloadProperties {
    unsigned int n_thread;
    unsigned int n_process;
    unsigned int n_seconds;
};

static WorkloadProperties WP = {
    .n_thread = 2,
    .n_process = 2
};

struct Thread {
    pthread_t thread;
    unsigned int thread_index;
    void *result;
    volatile int *data;
};

void *thread_main(void *tstate)
{
    Thread *t = (Thread *)tstate;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    printf("[%u.%u] thread %u\n", (int)getpid(), gettid(), t->thread_index);
    for (;;) {
        switch (t->thread_index) {
            case 0:
                t->data += 1;
                break;
            case 1:
                t->data += 8;
                break;
            case 2:
                t->data += 3;
                break;
            default:
                t->data += 4;
                break;
        }
        pthread_yield();
    }
    return NULL;
}

void process_main(void)
{
    printf("[%u] main=%p\n", getpid(), &process_main);
    pthread_attr_t attr;    
    Thread *threads = new Thread[WP.n_thread];
    pthread_attr_init(&attr);
    for (int i = 0; i < WP.n_thread; ++i) {
        Thread *t = &threads[i];
        t->thread_index = i;
        pthread_create(&t->thread, &attr, thread_main, t);
        printf("[%u] created thread #%u\n", getpid(), i);
    }
    time_t t0 = time(NULL);
    while (time(NULL) <= t0 + WP.n_seconds);
    printf("Cancelling...\n");
    for (int i = 0; i < WP.n_thread; ++i) {
        Thread *t = &threads[i];        
        pthread_cancel(t->thread);
        pthread_join(t->thread, &t->result);
    }
    delete [] threads;
}

int main(int argc, char **argv)
{
    printf("\n[%u] ** PERF WORKLOAD **\n\n", getpid());
    WP.n_process = atoi(argv[1]);
    WP.n_thread = atoi(argv[2]);
    WP.n_seconds = atoi(argv[3]);
    if (WP.n_process == 1) {        
        process_main();
    } else {
        pid_t *procs = new pid_t[WP.n_process];
        for (int i = 0; i < WP.n_process; ++i) {
            pid_t p = fork();
            if (!p) {
                process_main();                
            } else {
                printf("[%u] forked %u\n", getpid(), p);
            }
        }
        for (int i = 0; i < WP.n_process; ++i) {
            int status;
            waitpid(procs[i], &status, 0);
        }
    }
    return 0;
}