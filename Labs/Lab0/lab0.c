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

#define SIZE 10000
static int dcflag = 0;
static int catchflag = 0;
// register a SIGSEGV handler that catches the segmentation fault
void sigsegv_handler() {
    // catch a seg fault when no core dumped
    if (catchflag == 1 && dcflag == 0) {
        fprintf(stderr, "SIGSEGV caught\n");
        exit(4);
    }
    // force a seg fault otherwise
    if (dcflag == 1) {
        char* s1 = NULL;
        char s2 = *s1;
        s2 = s2;
    }
}

int main(int argc, char* argv[]) {
    /*
    struct option {
        const char *name;
        int has_arg; -> no_argument, required_argument, optional argument
        int *flag; -> NULL returns val, otherwise 0
        int val;
    }
    */

    static struct option longopts[] = { 
        { "input", required_argument, NULL, 'i' },
        { "output", required_argument, NULL, 'o' },
        { "segfault", no_argument, NULL, 's' },
        { "catch", no_argument, NULL, 'c' },
        { "dump-core", no_argument, NULL, 'd'},
        { 0, 0, 0, 0 }
    };

    /* declare varaibles */
    int ifd = 0;
    int ofd = 0;
    int sfflag = 0;
    char opt = '0';
    char buffer[SIZE];
    char* seg1 = NULL;
    char seg2 = '0';


    while (1) {
        opt = getopt_long(argc, argv, "i:o:scd", longopts, NULL);
        if (opt == -1)
            break;
        switch(opt) {
            case 'i':
            case 'o':
            case 'c':
            case 'd':
            case 's':
                break;
            default:
                fprintf(stderr, "Usage: %s\n", argv[0]);
                fprintf(stderr, "--input=[input_file] : specify an input file\n");
                fprintf(stderr, "--output=[output_file] : specify an output file\n");
                fprintf(stderr, "--segfault : force a segmentation fault\n");
                fprintf(stderr, "--catch : catch a sementation fault\n");
                fprintf(stderr, "--dump-core : dump core on segmentation faults\n");
                exit(1);
        }
    }

    // there are arguments left which are neither option nor option arguments
    if (argv[optind] != NULL) {
        fprintf(stderr, "Syntax error: '%s'\n", argv[optind]);
        fprintf(stderr, "--input=[input_file] : specify an input file\n");
        fprintf(stderr, "--output=[output_file] : specify an output file\n");
        fprintf(stderr, "--segfault : force a segmentation fault\n");
        fprintf(stderr, "--catch : catch a sementation fault\n");
        fprintf(stderr, "--dump-core : dump core on segmentation faults\n");
        exit(1);
    }

    optind = 1;

    while (1) {
        opt = getopt_long(argc, argv, "i:o:scd", longopts, NULL);
        if (opt == -1)
            break;

        switch (opt) {
            case 'i':
                ifd = open(optarg, O_RDONLY);
                if (!optarg) {
                    fprintf(stderr, "--input error: can't open file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    exit(2);
                }
                // direct to the input file normally
                if (ifd >= 0) {
                    close(0);
                    dup(ifd);
                    close(ifd);
                }
                // If you are unable to open the specified input file, exit(2) with status 1.
                else {
                    fprintf(stderr, "--input error: can't open file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    exit(2);
                }
                break;

            case 'o':
                // If the file already exists, truncate it to zero size. 
                ofd = creat(optarg, 0666);  //rw-rw-rw-
                // ofd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 0666);

                if (!optarg) {
                    fprintf(stderr, "--output error: can't open file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    exit(3);
                }
                // direct to the output normally
                if (ofd >= 0) {
                    close(1);
                    dup(ofd);
                    close(ofd);
                }
                // If you are unable to create or truncate the specified output file, exit with status 2.
                else {
                    fprintf(stderr, "--output error: can't create the output file %s. ", optarg);
                    fprintf(stderr, "%s\n", strerror(errno));
                    exit(3);
                }
                break;

            case 's':
                // force a segfault
                seg2 = *seg1;
                seg2 = seg2;
                sfflag = 1;
                break;

            case 'c':
                // catch a segfault
                catchflag = 1;
                dcflag = 0;
                // catch action is executed in the end
                signal(SIGSEGV, sigsegv_handler);
                break;

            case 'd':
                catchflag = 0;
                dcflag = 1;
                break;

            case ':':
                printf("Missing arg\n");
                break;

            default:
                // unrecognized argument
                fprintf(stderr, "Usage: %s\n", argv[0]);
                fprintf(stderr, "--input=[input_file] : specify an input file\n");
                fprintf(stderr, "--output=[output_file] : specify an output file\n");
                fprintf(stderr, "--segfault : force a segmentation fault\n");
                fprintf(stderr, "--catch : catch a sementation fault\n");
                fprintf(stderr, "--dump-core : dump core on segmentation faults\n");
                exit(1);
        }
    }

    if (sfflag == 1) {
        seg2 = *seg1;
        seg2 = seg2;
    }
    else {
        while(1) {
            int r = read(0, &buffer, SIZE);
            if (r < 0) {
                fprintf(stderr, "Error: can't read from the input\n");
                exit(errno);
            }
            else if (r == 0)
                exit(0);
            else {
                int w = write(1, &buffer, r);
                if (w < 0) {
                    fprintf(stderr, "Error: can't write to the output\n");
                    exit(errno);
                }
            }
        }
    }


    exit(0);
}

