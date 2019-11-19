#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>


/* global variables */
long long counter = 0;
int opt_yield = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int sl = 0;

/* basic add routine */
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield){
        sched_yield();
    }
    *pointer = sum;
}



/* thread routine helper */
void thread_add_helper(int niterations, int s_o, int x) {
    for (int i = 0; i < niterations; i++) {
        switch(s_o) {

            case 0: {
                add(&counter, x);
                break;
            }


            //mutex
            case 'm': {
                
                if (pthread_mutex_lock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(errno));
                    exit(1);
                }

                add(&counter, x);

                if (pthread_mutex_unlock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(errno));
                    exit(1);
                }

                break;
            }


            //spin lock
            case 's': {
                while (__sync_lock_test_and_set(&sl, 1));
                add(&counter, x);
                __sync_lock_release(&sl);
                break;
            }

            //compare and swap
            case 'c': {
                int newv = 0;
                int oldv = 0;
                //A full memory barrier is created when this function is invoked.
                do {
                    oldv = counter;
                    newv = oldv + x;
                } while ((__sync_val_compare_and_swap(&counter, oldv, newv)) != oldv);
                break;
            }
        }
        
    }
}

/* thread routine */
void* thread_add(int thread_args[]) {
    //int args[2] = thread_args;
    int niterations = thread_args[0];
    int sync_f = thread_args[1];
    thread_add_helper(niterations, sync_f, 1);
    thread_add_helper(niterations, sync_f, -1);
    return NULL;
}




/* helper functions */
int strtoint(char* optarg) {
    char* endptr = NULL;
    int ret = (int) strtol(optarg, &endptr, 10);
    if (endptr == optarg || ret < 0)
        ret = 1;
    return ret;
}


void diff(struct timespec* s, struct timespec* e, struct timespec* res) {
    if (e->tv_nsec < s->tv_nsec) {
        res->tv_sec = e->tv_sec - s->tv_sec - 1;
        res->tv_nsec = e->tv_nsec+1000000000 - s->tv_nsec;
    }
    else {
        res->tv_sec = e->tv_sec - s->tv_sec;
        res->tv_nsec = e->tv_nsec - s->tv_nsec;
    }
    fprintf(stdout, "start time: %ld\n", s->tv_sec*1000000000+s->tv_nsec);
    fprintf(stdout, "end time: %ld\n", e->tv_sec*1000000000+e->tv_nsec);
}





int main(int argc, char* argv[]) {
    /* declare options */
    static struct option long_options[] = {
        { "iterations", optional_argument, NULL, 'i' },
        { "threads", optional_argument, NULL, 't' },
        { "sync", required_argument, NULL, 's' },
        { "yield", no_argument, NULL, 'y' },
        { 0,0,0,0 }
    };


    /* declare variables*/
    int opt = 0;
    int sync_opt = 0;
    char test_name[20];
    int niterations = 1;
    int nthreads = 1;
    int noperations = 0;
    int t_args[2];
    struct timespec tp_start_main;
    struct timespec tp_end_main;
    long long rt_ntime;
    long long avg_ntime;

    int mutexflag = 0;
    int spinflag = 0;
    int cflag = 0;
    




    while(1) {
        opt = getopt_long(argc, argv, "", long_options, NULL);
        if (opt == -1)
            break;


        switch(opt) {

            /* --iterations=# */
            case 'i': {
                if (!optarg)
                    nthreads = 1;
                
                niterations = strtoint(optarg);

                break;
            }


            /* --threads=# */
            case 't': {
                if (!optarg)
                    nthreads = 1;

                nthreads = strtoint(optarg);

                break;
            }

            /* --sync=# */
            case 's': {
                sync_opt = optarg[0];
                switch(sync_opt) {
                    case 'm':
                        mutexflag = 1;
                        break;
                    case 's':
                        spinflag = 1;
                        break;
                    case 'c':
                        cflag = 1;
                        break;
                    default:
                        fprintf(stderr, "Unrecognized sync option %c\n", sync_opt);
                        exit(1);
                }
                break;
            }

            /* --yield */
            case 'y': {
                opt_yield = 1;
                break;
            }

            default: {
                fprintf(stderr, "Unrecognized argument: %s\n", argv[optind-1]);
                exit(1);
            }
        }
    }

    /* test names */
    if (opt_yield) {
        if (!mutexflag && !spinflag && !cflag)
            strcpy(test_name, "add-yield-none");
        else if (mutexflag)
            strcpy(test_name, "add-yield-m");
        else if (spinflag)
            strcpy(test_name, "add-yield-s");
        else if (cflag)
            strcpy(test_name, "add-yield-c");
    }
    else {
        if (!mutexflag && !spinflag && !cflag)
            strcpy(test_name, "add-none");
        else if (mutexflag)
            strcpy(test_name, "add-m");
        else if (spinflag)
            strcpy(test_name, "add-s");
        else if (cflag)
            strcpy(test_name, "add-c");
    }

    pthread_t threads[nthreads];
    noperations = 2*nthreads*niterations;
    t_args[0] = niterations;
    t_args[1] = sync_opt;

    /* get starting time */
    if (clock_gettime(CLOCK_MONOTONIC, &tp_start_main) < 0) {
        fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
        exit(1);
    }

    /* create threads */
    for (int i = 0; i < nthreads; i++) {

        if (pthread_create(&threads[i], NULL,(void*)&thread_add, &t_args) != 0){
            fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
            exit(1);
        }
    }

    /* join threads */
    for (int i = 0; i < nthreads; i++) {

        void* retVal;
        if (pthread_join(threads[i], &retVal) != 0){
            fprintf(stderr, "pthread_join() error: %s\n", strerror(errno));
            exit(1);
        }

    }

    /* get ending time */
    if (clock_gettime(CLOCK_MONOTONIC, &tp_end_main) < 0) {
        fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
        exit(1);
    }




    rt_ntime = (tp_end_main.tv_sec - tp_start_main.tv_sec) * 1000000000;
    rt_ntime += tp_end_main.tv_nsec;
    rt_ntime -= tp_start_main.tv_nsec;

    if (noperations != 0)
        avg_ntime = rt_ntime / noperations;
    else
        avg_ntime = rt_ntime;


    /* print results to csv */
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", 
        test_name, nthreads, niterations, noperations, rt_ntime, avg_ntime, counter);
}