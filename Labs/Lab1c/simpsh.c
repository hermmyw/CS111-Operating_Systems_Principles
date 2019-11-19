//lab1c.c
// lab1b.c


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
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>


#define BUFSIZE 100
static int err = 0;
static struct rusage usage;
static struct rusage end_usage;

/* return true if the argument is a long option */
int is_long_option(char* opt) {
    if (strlen(opt) >= 2 && opt[0] == '-' && opt[1] == '-')
        return 1;
    return 0;
}

void sig_handler(int sig) {
    fprintf(stderr, "%d caught.\n", sig);
    exit(sig);
}

void profile(int pflag, struct rusage* start, char* option) {
    if (pflag == 1) {
        if (getrusage(RUSAGE_SELF, &end_usage) < 0) {
            fprintf(stderr, "getrusage() fails: %s\n", strerror(errno));
            if (err < 1) err = 1;
        }
        else if (start == NULL) {
            fprintf(stderr, "can't get starting time\n");
            if (err < 1) err = 1;
        }
        else {
            struct timeval usertime;
            struct timeval systime;
            timersub(&(end_usage.ru_utime), &(start->ru_utime), &usertime);
            timersub(&(end_usage.ru_stime), &(start->ru_stime), &systime);
            fprintf(stdout, "\'%s\'\t", option);
            fprintf(stdout, "user time: %ld.%06lds\t", usertime.tv_sec, usertime.tv_usec);
            fprintf(stdout, "system time: %ld.%06lds\n", systime.tv_sec, systime.tv_usec);
            fflush(stdout);
        }
    }
}

struct rusage start_profile(int pflag) {
    static struct rusage start_usage;

    if (pflag == 1) {
        if (getrusage(RUSAGE_SELF, &start_usage) < 0) {
            fprintf(stderr, "getrusage() fails: %s\n", strerror(errno));
            if (err < 1) err = 1;
        }
    }

    return start_usage;
}

int main(int argc, char* argv[]) {

    /* declare options and flags */
    static struct option longopts[] = {
        { "rdonly", required_argument, NULL, O_RDONLY },
        { "rdwr", required_argument, NULL, O_RDWR },
        { "wronly", required_argument, NULL, O_WRONLY },
        { "pipe", no_argument, NULL, 'p' },
        { "command", required_argument, NULL, 'c' },
        { "wait", no_argument, NULL, 't' },
        { "close", required_argument, NULL, 's' },
        { "verbose", no_argument, NULL, 'v' },
        { "profile", no_argument, NULL, 'o' },
        { "abort", no_argument, NULL, 'a' },
        { "catch", required_argument, NULL, 'h' },
        { "ignore", required_argument, NULL, 'i' },
        { "default", required_argument, NULL, 'f' },
        { "pause", no_argument, NULL, 'u' },


        // flags
        { "append", no_argument, NULL, O_APPEND },
        { "cloexec", no_argument, NULL, O_CLOEXEC },
        { "creat", no_argument, NULL, O_CREAT },
        { "directory", no_argument, NULL, O_DIRECTORY },
        { "dsync", no_argument, NULL, O_DSYNC },
        { "excl", no_argument, NULL, O_EXCL },
        { "nofollow", no_argument, NULL, O_NOFOLLOW },
        { "nonblock", no_argument, NULL, O_NONBLOCK },
        { "rsync", no_argument, NULL, O_RSYNC },
        { "sync", no_argument, NULL, O_SYNC },
        { "trunc", no_argument, NULL, O_TRUNC },
        { 0,0,0,0 }
    };

    /* declare variables */
    int opt = 0;
    int brk = 0;
    int vflag = 0;
    int crtflag = 0;
    int pflag = 0;
    int cmdind = 0;
    int flag = 0;
    int sig = 0;
    int ifd = 0;

    /* file descriptors */
    int f_ind = 0;
    int* fds;
    int NFILES = 20;
    fds = malloc(NFILES*sizeof(int));
    if (!fds) {
        fprintf(stderr, "fds[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }
    int* file_valid;
    file_valid = malloc(NFILES*sizeof(int));
    if (!file_valid) {
        fprintf(stderr, "fds[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }
    for (int i = 0; i < NFILES; i++) {
        file_valid[i] = 1;
    }

    /* file numbers for subcommand */
    char* subcmd;
    int* subcmd_files;
    subcmd_files = malloc(3*sizeof(int));
    if (!subcmd_files) {
        fprintf(stderr, "subcmd_files[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }
    char** subarg_buf = malloc(sizeof(char*));
    if (!subarg_buf) {
        fprintf(stderr, "subarg_buf[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }
    

    /* processes */
    int nProcesses = 0;
    int* processes = malloc(sizeof(int));
    int* processes_args_ind = malloc(sizeof(int));

    if (!processes) {
        fprintf(stderr, "processes[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }

    if (!processes_args_ind) {
        fprintf(stderr, "processes[] malloc() fails: %s\n", strerror(errno));
        exit(-1);
    }



    /* handling errors */
    //int err = 0;
    int status = 0;
    int exit_status = 0;



    /* creating pipes */
    int pipefd[2];
    

    /* usage */
    //struct rusage usage;




    while(1) {


        opt = getopt_long(argc, argv, "", longopts, NULL);
        if (opt == -1)
            break;

        switch(opt) {


            /* --verbose */
            case 'v':

                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s\n", argv[optind-1]);
                    fflush(stdout);
                }
                vflag = 1;

                profile(pflag, &usage, argv[optind-1]);

                break;


            /* flags */
            case O_CREAT: // mode must be supplied when O_CREAT or O_TMPFILE is specified in flags
                crtflag = 1;
            case O_APPEND:
            case O_CLOEXEC: // close after execution
            case O_DIRECTORY: // fail if not a directory
            case O_DSYNC:
            case O_EXCL: // use with O_CREAT, not overwrite existing file
            case O_NOFOLLOW: //If pathname is a symbolic link, then the open fails, with the error ELOOP.
            case O_NONBLOCK:
            //case O_RSYNC:
            case O_SYNC:
            case O_TRUNC:
                if (vflag){
                    fprintf(stdout, "%s %s\n", argv[optind-1], optarg);
                    fflush(stdout);
                }
                flag = flag | opt;
                break;



            /* --rdonly, --wronly, --rdwr */
            case O_RDONLY:
            case O_WRONLY:
            case O_RDWR:
                usage = start_profile(pflag);


                if (vflag){
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }

                /* resize fd arrays */
                if (f_ind == NFILES) {
                    NFILES *= 2;
                    fds = realloc(fds, NFILES * sizeof(int));
                    file_valid = realloc(file_valid, NFILES * sizeof(int));
                    if (!fds || !file_valid) {
                        fprintf(stderr, "fds[] or file_valid[] realloc() fails: %s\n", strerror(errno));
                        exit(-1);
                    }
                    for (int i = NFILES/2; i < NFILES; i++) {
                        file_valid[i] = 1;
                    }                   
                }

                /* open file with the correct flag */
                flag = (flag | opt);

                if (crtflag == 1)
                    ifd = open(optarg, flag, 0644);
                else
                    ifd = open(optarg, flag);


                /* store the file descriptor */
                file_valid[f_ind] = 1;
                fds[f_ind++] = ifd;


                if (!optarg || ifd < 0) {
                    fprintf(stderr, "\'%s\': can't open file %s. ", argv[optind-2], optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    if (err < 1) err = 1;
                    file_valid[f_ind] = 0;
                }

                /* reset flag */
                flag = 0;

                // if (pflag) {
                //     if (getrusage(RUSAGE_SELF, &usage) < 0) {
                //         fprintf(stderr, "getrusage() fails: %s\n", strerror(errno));
                //         if (err < 1) err = 1;
                //     }
                //     else {
                //         //nd-2]);
                //         fprintf(stdout, "user time: %ld.%06ld\t", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
                //         fprintf(stdout, "system time: %ld.%06ld\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
                //         fflush(stdout);
                //     }
                // }
                profile(pflag, &usage, argv[optind-2]);

                break;


            

            /* --pipe */
            case 'p':
                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s\n", argv[optind-1]);
                    fflush(stdout);
                }


                // open two file numbers: read end and write end
                // data written to the write end of the pipe is buffered by the kernel
                // until it is read from the read end of the pipe.
                if (pipe(pipefd) < 0) {
                    fprintf(stderr, "pipe() fails: %s\n", strerror(errno));

                    // max error
                    if (err < 1) err = 1;
                }
                else {
                    if (f_ind == NFILES) {
                        NFILES *= 2;
                        fds = realloc(fds, NFILES * sizeof(int));
                        if (!fds) {
                            fprintf(stderr, "fds[] realloc() fails: %s\n", strerror(errno));
                            exit(-1);
                        }                   
                    }

                    /* increment two file descriptors */
                    file_valid[f_ind] = 1;
                    fds[f_ind++] = pipefd[0];
                    file_valid[f_ind] = 1;
                    fds[f_ind++] = pipefd[1];
                }

                /* output profile */
                profile(pflag, &usage, argv[optind-1]);

                break;






            /* --command i o e cmd agrs */
            case 'c':
                usage = start_profile(pflag);

                // find command index
                cmdind = optind-2;

                // move optind to the next long option
                while ((optind < argc) && (is_long_option(argv[optind]) == 0))
                    optind++;

                // --verbose
                if (vflag) {
                    fprintf(stdout, "--command ");


                    for (int i = cmdind+1; (i < argc) && is_long_option(argv[i]) == 0; i++) {
                        fprintf(stdout, "%s ", argv[i]);
                    }
                    fprintf(stdout, "\n");


                    // always flush stdout to avoid stdout hanging in buffer
                    fflush(stdout);
                }


                /* check if --command is followed by sufficient arguments */
                int i = cmdind+1;
                int cmdargc = 0;
                while ((i < argc) && (is_long_option(argv[i]) == 0)) {
                    i++;
                    cmdargc++;
                }
                if (cmdargc < 4) {
                    fprintf(stderr, "No enough arguments for --command: %d\n", cmdargc);
                    if (err < 1) err = 1;
                    /* output profile */
                    profile(pflag, &usage, argv[cmdind]);
                    break;
                }
                

                /* identify 3 file descriptors, continue to the next option if invalid file no. */
                char* endptr = NULL;
                for (int i = 1; i < 4; i++) {
                    subcmd_files[i-1] = (int) strtol(argv[cmdind+i], &endptr, 10);
                    if (endptr == argv[cmdind+i]) {
                        fprintf(stderr, "Please provide a valid file. %s is not a number.\n", argv[cmdind+i]);
                        if (err < 1) err = 1;
                        brk = 1;;
                    }
                }
                if (brk == 1) {
                    /* output profile */
                    profile(pflag, &usage, argv[cmdind]);
                    break;
                }

                for (int i = 0; i < 3; i++) {
                    if (subcmd_files[i] >= f_ind || subcmd_files[i] < 0) {
                        fprintf(stderr, "Please provide a valid file. %d is not a correct index.\n", subcmd_files[i]);
                        if (err < 1) err = 1;
                        brk = 1;
                    }
                }
                if (brk == 1) {
                    /* output profile */
                    profile(pflag, &usage, argv[cmdind]);
                    break;
                }

                /* check if the 3 files are accessible */
                for (int i = 0; i < 3; i++) {
                    if (file_valid[subcmd_files[i]] == 0) {
                        fprintf(stderr, "File %d is not valid.\n", subcmd_files[i]);
                        err = 1;
                    }
                }


                /* identify subcommand */
                subcmd = argv[cmdind+4];
                int si = cmdind+5;
                int subargc =  1;
                subarg_buf[0] = subcmd;
                



                /* store subcommand arguments into a buffer */
                while ((si < argc)) {
                    if (is_long_option(argv[si]) == 0) {
                        subarg_buf = realloc(subarg_buf, (subargc+1) * sizeof(char*));
                        if (!subarg_buf) {
                            fprintf(stderr, "subarg_buf[] realloc() fails: %s\n", strerror(errno));
                            exit(-1);
                        }                
                        subarg_buf[subargc++] = argv[si++];
                    }
                    else
                        break;
                }
                subarg_buf = realloc(subarg_buf, (subargc+1) * sizeof(char*));
                if (!subarg_buf) {
                    fprintf(stderr, "subarg_buf[] realloc() fails: %s\n", strerror(errno));
                    exit(-1);
                }
                subarg_buf[subargc] = NULL;
                



                /* start a new process to execute subcommand */
                if (subcmd && subarg_buf) {

                    pid_t pid = fork();
                    
                    if (pid < 0) {
                        fprintf(stderr, "Can't create a new process\n");
                        if (err < 1) err = 1;
                        /* output profile */
                        profile(pflag, &usage, argv[cmdind]);
                        break;
                    }

                    /* Child process */
                    else if (pid == 0) {
                        // fprintf(stderr, "subcmd_files[0]: %d\n", subcmd_files[0]);
                        // fprintf(stderr, "subcmd_files[1]: %d\n", subcmd_files[1]);
                        // fprintf(stderr, "subcmd_files[2]: %d\n", subcmd_files[2]);

                        /* redirect i/o */
                        if (dup2(fds[subcmd_files[0]], 0) < 0) {
                            fprintf(stderr, "dup() --command input failed. File index: %d\n", subcmd_files[0]);
                        }

                        if (dup2(fds[subcmd_files[1]], 1) < 0) {
                            fprintf(stderr, "dup() --command output failed. File index: %d\n", subcmd_files[1]);
                        }
                        
                        if (dup2(fds[subcmd_files[2]], 2) < 0) {
                            fprintf(stderr, "dup() --command error failed. File index: %d\n", subcmd_files[2]);
                        }


                        /* close files to avoid hanging */
                        for (int i = 0; i < f_ind; i++) {
                            close(fds[i]);
                            file_valid[i] = 0;
                        }


                        /* execute subcommand */
                        if (execvp(subarg_buf[0], subarg_buf) < 0) {
                            fprintf(stderr, "%s error: %s\n", subarg_buf[0], strerror(errno));
                        }


                    }

                    /* Parent process */
                    else {


                        /* store processes pids and arguments for each pid */
                        processes = realloc(processes, (nProcesses+1)*sizeof(int));
                        processes_args_ind = realloc(processes_args_ind, (nProcesses+1)*sizeof(int*));

                        if (!processes || !processes_args_ind) {
                            fprintf(stderr, "processes[] realloc() fails: %s\n", strerror(errno));
                            exit(-1);
                        }

                        processes[nProcesses] = pid;
                        processes_args_ind[nProcesses++] = cmdind+4;

                    }



                }
                /* output profile */
                profile(pflag, &usage, argv[cmdind]);
                break;

            

            /* --wait */
            case 't':
                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s\n", argv[optind-1]);
                    fflush(stdout);
                }

                fflush(stdout);

                i = 0;
                while(1) {

                    /* wait for all child processes to terminate */
                    pid_t p = waitpid(-1, &status, 0);


                    if (i >= nProcesses || p < 0)
                        break;

                    // returns true if the child terminated normally
                    if (WIFEXITED(status)) {
                        
                        // returns the exit status of the child
                        exit_status = WEXITSTATUS(status);
                        
                        fprintf(stdout, "exit %d ", exit_status);

                    }


                    // returns true if the child terminated with a signal
                    else if (WIFSIGNALED(status)) {

                        // returns the signal of the child 
                        exit_status = WTERMSIG(status);
                        
                        fprintf(stdout, "signal %d ", exit_status);

                        // exit status is (128 + signal)
                        exit_status += 128;

                    }


                    else {
                        fprintf(stderr, "Child exit status is unknown\n");
                        /* output profile */
                        profile(pflag, &usage, argv[optind-1]);
                        break;
                    }


                    // highest exit code
                    if (exit_status > err)
                        err = exit_status;


                    /* print the argumetns of the child process program */
                    int pro_ind = 0;
                    for (int i = 0; i < nProcesses; i++) {
                        if (processes[i] == p) {

                            // process index to search for argument position in argv[]:
                            pro_ind = i; 
                            break;

                        }
                    }


                    for (int j = processes_args_ind[pro_ind]; j < argc; j++) {
                        if (is_long_option(argv[j]))
                            break;
                        fprintf(stdout, "%s ", argv[j]);
                    }
                    fprintf(stdout, "\n");
                    fflush(stdout);



                    i++;




                    
                }
                /* output profile */
                if (pflag == 1) {
                    struct rusage child_u;
                    if (getrusage(RUSAGE_CHILDREN, &child_u) < 0) {
                        fprintf(stderr, "getrusage() fails: %s\n", strerror(errno));
                        if (err < 1) err = 1;
                    }
                    else {
                        fprintf(stdout, "children sum: \t");
                        fprintf(stdout, "user time: %ld.%06lds\t", child_u.ru_utime.tv_sec, child_u.ru_utime.tv_usec);
                        fprintf(stdout, "system time: %ld.%06lds\n", child_u.ru_stime.tv_sec, child_u.ru_stime.tv_usec);
                        fflush(stdout);
                    }
                }
                profile(pflag, &usage, argv[optind-1]);

                break;




            /* --close N */
            case 's':
                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }

                if (!optarg) {
                    fprintf(stderr, "Please provide a valid file index\n");
                    if (err < 1) err = 1;
                    break;
                }

                /* convert string to int */
                endptr = NULL;
                int n = (int) strtol(optarg, &endptr, 10);
                if (endptr == optarg) {
                    fprintf(stderr, "Please provide a valid file index. %s is not a number.\n", optarg);
                    if (err < 1) err = 1;
                    break;
                }

                /* file index needs to be in range */
                if (n < f_ind && n >= 0) {
                    close(fds[n]);

                    // IMPORTANT: invalidate the file
                    file_valid[n] = 0;
                }

                else {
                    fprintf(stderr, "Please provide a valid file index. %s is out of range.\n", optarg);
                    if (err < 1) err = 1;
                }

                /* output profile */
                profile(pflag, &usage, argv[optind-2]);
                break;





            /* --profile */
            case 'o':
                if (vflag) {
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }
                pflag = 1;
                break;






            /* --abort */
            case 'a':
                if (vflag) {
                    fprintf(stdout, "%s\n", argv[optind-1]);
                    fflush(stdout);
                }

                fflush(stdout);

                /* force a segmentation fault */
                char* a = NULL;
                *a = 0;

                break;


            

            /* --catch N */
            case 'h':
                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }
                if (!optarg) {
                    fprintf(stderr, "Please provide a valid file index\n");
                    if (err < 1) err = 1;
                    break;
                }

                 /* convert string to int */
                endptr = NULL;
                sig = (int) strtol(optarg, &endptr, 10);
                if (endptr == optarg) {
                    fprintf(stderr, "Please provide a valid signal. %s is not a number.\n", optarg);
                    if (err < 1) err = 1;
                    break;
                }


                signal(sig, sig_handler);
                /* output profile */
                profile(pflag, &usage, argv[optind-2]);

                break;




            /* --ignore N */
            case 'i':
                usage = start_profile(pflag);

                if (vflag) {
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }
                if (!optarg) {
                    fprintf(stderr, "Please provide a valid file index\n");
                    if (err < 1) err = 1;
                    break;
                }

                

                /* convert string to int */
                endptr = NULL;
                sig = (int) strtol(optarg, &endptr, 10);
                if (endptr == optarg) {
                    fprintf(stderr, "Please provide a valid signal. %s is not a number.\n", optarg);
                    if (err < 1) err = 1;
                    break;
                }

                signal(sig, SIG_IGN);
                /* output profile */
                profile(pflag, &usage, argv[optind-2]);

                break;




            /* --default N */
            case 'f':
                usage = start_profile(pflag);
                if (vflag) {
                    fprintf(stdout, "%s %s\n", argv[optind-2], optarg);
                    fflush(stdout);
                }
                if (!optarg) {
                    fprintf(stderr, "Please provide a valid file index\n");
                    if (err < 1) err = 1;
                    break;
                }

                /* convert string to int */
                endptr = NULL;
                sig = (int) strtol(optarg, &endptr, 10);
                if (endptr == optarg) {
                    fprintf(stderr, "Please provide a valid signal. %s is not a number.\n", optarg);
                    if (err < 1) err = 1;
                    break;
                }


                signal(sig, SIG_DFL);
                /* output profile */
                profile(pflag, &usage, argv[optind-2]);

                break;



            /* --pause */
            case 'u':
                usage = start_profile(pflag);

                pause();
                /* output profile */
                profile(pflag, &usage, argv[optind-1]);

                break;



            default:
                fprintf(stderr, "Error: unrecognized option \'%s\'\n", argv[optind-1]);
                if (err < 1) err = 1;
                break;

        }
    }
    exit(err);
}