#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include "SortedList.h"

/* global variables */
int opt_yield = 0;
int nlists = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int sl = 0;
static int niterations = 1;
static int nthreads = 1;
static int sync_opt = 0;
static SortedList_t* list;  // head of sublists
static SortedList_t** sublists;  // insert to each sublist
static SortedListElement_t** elements_list;
struct timespec tp_start_lock;
struct timespec tp_end_lock;
long long* wait_time;


/* hash function */
unsigned int hash(const char* key) {
    unsigned int hash_n = 5381;
    int c;

    while ((c = *key++))
        hash_n = ((hash_n << 5) + hash_n) + c; /* hash * 33 + c */

    return hash_n;
}

/* handling segmentation fault */
void signal_handler() {
    fprintf(stderr, "Caught segmentation fault\n");
    exit(2);
}

/* thread routine helper */
/* inserts them all into multiple sublists */
void thread_list_insert(int t_ID) {

    for (int i = t_ID; i < niterations*nthreads; i+=nthreads) {
        int list_ind = hash(elements_list[i]->key) % nlists;
        
        switch(sync_opt) {
            case 0: {
                SortedList_insert(sublists[list_ind], elements_list[i]);
                break;
            }


            //mutex
            case 'm': {

                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                if (pthread_mutex_lock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(errno));
                    exit(1);
                }
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;
                

                SortedList_insert(sublists[list_ind], elements_list[i]);


                if (pthread_mutex_unlock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(errno));
                    exit(1);
                }
                break;
            }

            //spin lock
            case 's': {
                
                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                while (__sync_lock_test_and_set(&sl, 1));
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;

                SortedList_insert(sublists[list_ind], elements_list[i]);

                __sync_lock_release(&sl);

                break;
            }
            default: {
                fprintf(stderr, "Wrong option of sync: %c\n", sync_opt);
                break;
            }
        }



    }
}


/* checks the list length */
void thread_list_length(int t_ID) {
    
    for (int i = 0; i < nlists; i++) {

        switch(sync_opt) {
            case 0: {
                int l = SortedList_length(sublists[i]);
                if (l < 0) {
                    fprintf(stderr, "Corrupted list with length: %d\n", l);
                    exit(2);
                }
                break;
            }


            //mutex
            case 'm': {

                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                if (pthread_mutex_lock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(errno));
                    exit(1);
                }
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;
                
                int l = SortedList_length(sublists[i]);
                if (l < 0) {
                    fprintf(stderr, "Corrupted list with length: %d\n", l);
                    exit(2);
                }

                if (pthread_mutex_unlock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(errno));
                    exit(1);
                }
                break;
            }

            //spin lock
            case 's': {
                
                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                while (__sync_lock_test_and_set(&sl, 1));
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;

                int l = SortedList_length(sublists[i]);
                if (l < 0) {
                    fprintf(stderr, "Corrupted list with length: %d\n", l);
                    exit(2);
                }

                __sync_lock_release(&sl);

                break;
            }
            default: {
                fprintf(stderr, "Wrong option of sync: %c\n", sync_opt);
                break;
            }
        }



    }




}

/* looks up and deletes each of the keys it had previously inserted */
void thread_list_del(int t_ID) {

    for (int i = t_ID; i < niterations*nthreads; i+=nthreads) {
        int list_ind = hash(elements_list[i]->key) % nlists;


        switch(sync_opt) {
            case 0: {
                SortedListElement_t* del = SortedList_lookup(sublists[list_ind], elements_list[i]->key);
                if (!del) {
                    fprintf(stderr, "Lookup failed\n");
                    exit(2);
                }
                if (SortedList_delete(del) == 1) {
                    fprintf(stderr, "Deletion failed\n");
                    exit(2);
                }
                break;
            }


            //mutex
            case 'm': {

                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                if (pthread_mutex_lock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(errno));
                    exit(1);
                }
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;
                
                SortedListElement_t* del = SortedList_lookup(sublists[list_ind], elements_list[i]->key);
                if (!del) {
                    fprintf(stderr, "Lookup failed\n");
                    exit(2);
                }
                if (SortedList_delete(del) == 1) {
                    fprintf(stderr, "Deletion failed\n");
                    exit(2);
                }

                if (pthread_mutex_unlock(&lock) != 0) {
                    fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(errno));
                    exit(1);
                }
                break;
            }

            //spin lock
            case 's': {
                
                /* get starting time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_start_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                while (__sync_lock_test_and_set(&sl, 1));
                /* get ending time */
                if (clock_gettime(CLOCK_MONOTONIC, &tp_end_lock) < 0) {
                    fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
                    exit(1);
                }
                wait_time[t_ID] += (tp_end_lock.tv_sec - tp_start_lock.tv_sec) * 1000000000 + tp_end_lock.tv_nsec - tp_start_lock.tv_nsec;

                SortedListElement_t* del = SortedList_lookup(sublists[list_ind], elements_list[i]->key);
                if (!del) {
                    fprintf(stderr, "Lookup failed\n");
                    exit(2);
                }
                if (SortedList_delete(del) == 1) {
                    fprintf(stderr, "Deletion failed\n");
                    exit(2);
                }

                __sync_lock_release(&sl);

                break;
            }
            default: {
                fprintf(stderr, "Wrong option of sync: %c\n", sync_opt);
                break;
            }
        }
    }
}
/* thread routine */


void* thread_list(int* t_ID) {

    int t = *t_ID;

    thread_list_insert(t);
    thread_list_length(t);
    thread_list_del(t);

    pthread_exit(NULL);
}




/* helper functions */
int strtoint(char* optarg) {
    char* endptr = NULL;
    int ret = (int) strtol(optarg, &endptr, 10);
    if (endptr == optarg || ret < 0)
        ret = 1;
    return ret;
}

int main(int argc, char* argv[]) {
    /* handle segmentation fault */
    signal(SIGSEGV, signal_handler);

    /* declare options */
    static struct option long_options[] = {
        { "iterations", optional_argument, NULL, 'i' },
        { "threads", optional_argument, NULL, 't' },
        { "sync", required_argument, NULL, 's' },
        { "yield", required_argument, NULL, 'y' },
        { "lists", required_argument, NULL, 'l' },
        { 0,0,0,0 }
    };


    /* declare variables*/
    int opt = 0;
    //int sync_opt = 0;
    char test_name[20];
    strcpy(test_name, "list");
    long noperations = 0;
    struct timespec tp_start_main;
    struct timespec tp_end_main;
    long long rt_ntime;
    long long avg_ntime;
    long long avg_wtime = 0;
    int yflag = 0;
    char* yield_str = malloc(6);
    char* sync_str;




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
                if (!optarg)
                    sync_opt = 0;
                else
                    sync_opt = optarg[0];

                switch(sync_opt) {
                    case 'm': {
                        //mutexflag = 1;
                        sync_str = "-m";
                        break;
                    }
                    case 's':
                        //spinflag = 1;
                        sync_str = "-s";
                        break;
                    case 'c':
                        //cflag = 1;
                        sync_str = "-c";
                        break;

                    default:
                        fprintf(stderr, "Unrecognized sync option %c\n", sync_opt);
                        exit(1);
                }
                break;
            }

            /* --yield */
            case 'y': {
                yflag = 1;

                if (!optarg)
                    yield_str = strcpy(yield_str, "-none");

                else if (strlen(optarg) > 4) {
                    fprintf(stderr, "Wrong option of yield: %s\n", optarg);
                    exit(1);
                }

                else {
                    yield_str[0] = '-';
                    int index = 1;
                    for (int i = 0; i < 3; i++) {
                        if (optarg[i] == 'i') {
                            opt_yield |= INSERT_YIELD;
                            yield_str[index++] = 'i';
                        }
                        else if (optarg[i] == 'd') {
                            opt_yield |= DELETE_YIELD;
                            yield_str[index++] = 'd';
                        }
                        else if (optarg[i] == 'l') {
                            opt_yield |= LOOKUP_YIELD;
                            yield_str[index++] = 'l';
                        }
                    }
                    yield_str[index] = '\0';
                }
                break;
            }

            /* --lists=# */
            case 'l': {

                if (!optarg) {
                    fprintf(stderr, "Require argument for --lists\n");
                    exit(1);
                }

                nlists = strtoint(optarg);

                break;
            }


            default: {
                fprintf(stderr, "Unrecognized argument: %s\n", argv[optind-1]);
                exit(1);
            }
        }
    }


    /* test names */
    if (yflag == 0)
        yield_str = strcpy(yield_str, "-none");
    if (sync_opt == 0)
        sync_str = "-none";
    strcat(test_name, yield_str);
    strcat(test_name, sync_str);



    /* initialize an empty list */
    list = malloc(sizeof(SortedList_t)); 
    if (list == NULL) {
        fprintf(stderr, "Can't initialize a list\n");
        exit(1);
    }
    list->key = NULL;
    list->next = list;
    list->prev = list;

    /* initialize empty sublists */
    sublists = malloc(nlists*sizeof(SortedList_t*));
    for (int i = 0; i < nlists; i++) {
        sublists[i] = malloc(sizeof(SortedList_t));
        if (!sublists[i]) {
            fprintf(stderr, "Can't initialize sublists\n");
            exit(1);
        }
        sublists[i]->key = NULL;
        sublists[i]->next = sublists[i];
        sublists[i]->prev = sublists[i];
    }

    /* creates and initialize with random keys the required number of list elements */
    char* charset = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };
    
    elements_list = malloc(nthreads*niterations*sizeof(SortedListElement_t*));
    for (int i = 0; i < nthreads*niterations; i++) {
        elements_list[i] = malloc(sizeof(SortedListElement_t));
        if (!elements_list[i]) {
            fprintf(stderr, "Can't initialize elements\n");
            exit(1);
        }
        char* str = malloc(sizeof(char)*2);
        int r = rand() % 51;
        str[0] = charset[r];
        str[1] = '\0';
        elements_list[i]->key = str;
        elements_list[i]->prev = NULL;
        elements_list[i]->next = NULL;
    }


    /* initialize thread arguments */
    pthread_t threads[nthreads];
    noperations = 3*nthreads*niterations;
    int thread_IDs[nthreads];
    wait_time = malloc(nthreads*sizeof(long long));
    for (int i = 0; i < nthreads; i++) {
        wait_time[i] = 0;
    }

    /* get starting time */
    if (clock_gettime(CLOCK_MONOTONIC, &tp_start_main) < 0) {
        fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
        exit(1);
    }

    /* create threads */
    for (int i = 0; i < nthreads; i++) {
        thread_IDs[i] = i;
        if (pthread_create(&threads[i], NULL, (void*)&thread_list, &thread_IDs[i]) != 0){
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


    /* Check final length */
    opt_yield = 0;
    int length = SortedList_length(list);
    if (length != 0) {
        fprintf(stderr, "Corrupted list with length: %d\n", length);
        fprintf(stderr, "\t caused by %s, %d threads and %d iterations\n", test_name, nthreads, niterations);

        /* free memory */
        for (int i = 0; i < nthreads*noperations; i++) {
            free(elements_list[i]);
        }
        free(elements_list);
        exit(2);
    }


    /* computer average wait time */ 
    if (sync_opt != 0) {
        long long wait_total = 0;
        for (int i = 0; i < nthreads; i++) {
            wait_total += wait_time[i];
        }
        avg_wtime = wait_total / noperations;
    }

    /* print results to csv */
    fprintf(stdout, "%s,%d,%d,%d,%ld,%lld,%lld,%lld\n", 
        test_name, nthreads, niterations, nlists, noperations, rt_ntime, avg_ntime, avg_wtime);


    /* free memory */
    for (int i = 0; i < nthreads*niterations; i++) {
        if (elements_list) {
            //free((void* )elements_list[i]->key);
            free(elements_list[i]);
        }
    }
    free(elements_list);
    free(list);
    free(yield_str);





}