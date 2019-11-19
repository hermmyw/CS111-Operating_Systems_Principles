// lab1a.c

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE 100

int is_long_option(char* opt) {
    if (strlen(opt) >= 2 && opt[0] == '-' && opt[1] == '-')
        return 1;
    return 0;
}

int main(int argc, char* argv[]) {

    /* declare options */
    static struct option longopts[] = {
        { "rdonly", required_argument, NULL, 'r' },
        { "wronly", required_argument, NULL, 'w' },
        { "verbose", no_argument, NULL, 'v' },
        { "command", required_argument, NULL, 'c' },
        { 0,0,0,0 }
    };

    /* declare variables */
    char opt = '0';
    int brk = 0;
    int vflag = 0;
    int cmdind = 0;

    /* file descriptors */
    int f_ind = 0;
    int* fds;
    int NFILES = 10;
    fds = malloc(NFILES*sizeof(int));
    if (!fds) {
        fprintf(stderr, "fds[] malloc() fails: %s\n", strerror(errno));
        exit(1);
    }

    /* file numbers for subcommand */
    char* subcmd;
    int* subcmd_files;
    subcmd_files = malloc(3*sizeof(int));
    if (!subcmd_files) {
        fprintf(stderr, "subcmd_files[] malloc() fails: %s\n", strerror(errno));
        exit(1);
    }
    char** subarg_buf = malloc(sizeof(char*));
    if (!subarg_buf) {
        fprintf(stderr, "subarg_buf[] malloc() fails: %s\n", strerror(errno));
        exit(1);
    }

    /* processes */
    int nProcesses = 0;
    int* processes = malloc(sizeof(int));
    if (!processes) {
        fprintf(stderr, "processes[] malloc() fails: %s\n", strerror(errno));
        exit(1);
    }

    /* handling errors */
    int err = 0;

    while(1) {
        //printf("optind: %d\n", optind);
        opt = getopt_long(argc, argv, "", longopts, NULL);
        if (opt == -1)
            break;

        switch(opt) {
            case 'v':
                if (vflag) {
                    fprintf(stdout, "--verbose\n");
                }
                vflag = 1;
                break;


            case 'r':
                if (vflag)
                    fprintf(stdout, "--rdonly %s\n", optarg);

                if (f_ind == NFILES) {
                    NFILES *= 2;
                    fds = realloc(fds, NFILES * sizeof(int));
                    if (!fds) {
                        fprintf(stderr, "fds[] realloc() fails: %s\n", strerror(errno));
                        exit(1);
                    }                   
                }

                int ifd = open(optarg, O_RDONLY);
                fds[f_ind++] = ifd;
                
                if (!optarg || ifd < 0) {
                    fprintf(stderr, "\'--rdonly\': can't open file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    err = 1;
                }
                break;


            case 'w':
                if (vflag)
                    fprintf(stdout, "--wronly %s\n", optarg);

                if (f_ind == NFILES) {
                    NFILES *= 2;
                    fds = realloc(fds, NFILES * sizeof(int));
                    if (!fds) {
                        fprintf(stderr, "fds[] realloc() fails: %s\n", strerror(errno));
                        exit(1);
                    }   
                }

                int ofd = open(optarg, O_WRONLY);  //-w--w--w-
                fds[f_ind++] = ofd;
                //printf("--ofd: %d\n", fds[f_ind-1]);
                if (!optarg || ofd < 0) {
                    fprintf(stderr, "\'--wronly\': can't open file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    err = 1;
                }
                break;


            case 'c':

                // find command index
                cmdind = optind-2;

                // move optind to the next long option
                while ((optind < argc) && (is_long_option(argv[optind]) == 0))
                    optind++;

                // --verbose
                if (vflag) {
                    fprintf(stdout, "--command ");
                    int i = cmdind+1;
                    while ((i < argc) && is_long_option(argv[i]) == 0) {
                        fprintf(stdout, "%s ", argv[i++]);
                    }
                    fprintf(stdout, "\n");
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
                    err = 1;
                    break;
                }
                

                /* identify 3 file descriptors, continue to the next option if invalid file no. */
                char* endptr = NULL;
                for (int i = 1; i < 4; i++) {
                    subcmd_files[i-1] = (int) strtol(argv[cmdind+i], &endptr, 10);
                    if (endptr == argv[cmdind+i]) {
                        fprintf(stderr, "Please provide a valid file. %s is not a number.\n", argv[cmdind+i]);
                        err = 1;
                        brk = 1;;
                    }
                }
                if (brk == 1)
                    break;

                for (int i = 0; i < 3; i++) {
                    if (subcmd_files[i] >= f_ind || subcmd_files[i] < 0) {
                        fprintf(stderr, "Please provide a valid file. %d is not a correct index.\n", subcmd_files[i]);
                        err = 1;
                        brk = 1;
                    }
                }
                if (brk == 1)
                    break;


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
                            exit(1);
                        }                
                        subarg_buf[subargc++] = argv[si++];
                    }
                    else
                        break;
                }
                subarg_buf = realloc(subarg_buf, (subargc+1) * sizeof(char*));
                if (!subarg_buf) {
                    fprintf(stderr, "subarg_buf[] realloc() fails: %s\n", strerror(errno));
                    exit(1);
                }
                subarg_buf[subargc] = NULL;


                /* start a new process to execute subcommand */
                if (subcmd && subarg_buf) {

                    pid_t pid = fork();
                    
                    if (pid < 0) {
                        fprintf(stderr, "Can't create a new process\n");
                        err = 1;
                        break;
                    }

                    /* Child process */
                    else if (pid == 0) {

                        /* redirect i/o */
                        if (dup2(fds[subcmd_files[0]], 0) < 0) {
                            fprintf(stderr, "Can't open --command input. File No.: %d\n", subcmd_files[0]);
                            err = 1;
                            break;
                        }

                        if (dup2(fds[subcmd_files[1]], 1) < 0) {
                            fprintf(stderr, "Can't open --command output. File No.: %d\n", subcmd_files[1]);
                            err = 1;
                            break;
                        }
                        
                        if (dup2(fds[subcmd_files[2]], 2) < 0) {
                            fprintf(stderr, "Can't open --command error. File No.: %d\n", subcmd_files[2]);
                            err = 1;
                            break;
                        }

                        /* close files to avoid hanging */
                        for (int i = 0; i < f_ind; i++) {
                            close(fds[i]);
                        }

                        /* execute subcommand */
                        if (execvp(subarg_buf[0], subarg_buf) < 0) {
                            // suberr = errno;
                            // if (suberr > err)
                            //     err = suberr;
                            err = 1;
                            fprintf(stderr, "%s error: %s\n", subarg_buf[0], strerror(errno));
                            break;
                        }

                    }

                    /* Parent process */
                    else {
                        processes = realloc(processes, (nProcesses+1)*sizeof(int));
                        if (!processes) {
                            fprintf(stderr, "processes[] realloc() fails: %s\n", strerror(errno));
                            exit(1);
                        }
                        processes[nProcesses++] = pid;
                    }



                }
                break;

            default:
                fprintf(stderr, "Error: unrecognized option \'%s\'\n", argv[optind-1]);
                err = 1;
                break;

        }
    }

    return err;
}