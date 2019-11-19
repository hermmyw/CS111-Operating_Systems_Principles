#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <mraa.h>
// #include <mraa/aio.h>
// #include <mraa/gpio.h>

#define AIO_PORT 1
#define GPIO_PIN 60
volatile sig_atomic_t flag = 1;

/* global variables */
float sample_rate = 1.0;
int logfd = -1;
int stop = 0;
int off = 0;
int tflag = 'F';
FILE* f_logfd;

void sig_handler(int signum)
{
    if (signum == SIGINT) {
        flag = 0;
    }
}

void button_handler() { flag = 0; }

int stoi(char* str) {
    char* endptr = NULL;
    int ret = (int) strtol(str, &endptr, 10);
    if (endptr == str || ret < 0) {
        fprintf(stderr, "Error: Please provide a valid sample rate. %s is not a number.\n", str);
        return -1;
    }
    return ret;
}

void shutdown() {
    /* get current time */ 
    struct timeval curr;
    if (gettimeofday(&curr, NULL) < 0) {
        fprintf(stderr, "Error: gettimeofday()\n");
        exit(1);
    }
    char timebuf[64];
    strftime(timebuf,64,"%T", localtime(&(curr.tv_sec)));

    fprintf(stdout, "%s SHUTDOWN\n", timebuf);
    if (logfd != -1) {
        fprintf(f_logfd, "%s SHUTDOWN\n", timebuf);
    }
    
    exit(0);
}

/* convert sensor temperature */
float c_to_f(float cdegree) { return cdegree*(9.0/5.0)+32.0; }

/* process input from terminal */
void execute(char* command, int nbytes) {
    if (nbytes <= 0)
        return;
    printf("Enter: %s\n", command);
    char cmd[256];
    memcpy(cmd, command, strlen(command));
    int i = 0;
    int j = 0;
    for (j = 0; j < nbytes; j++) {
        if (command[j] != '\n') continue;
        // report
        cmd[j] = '\0';
        if (logfd != -1) {
            fprintf(f_logfd, "%s\n", cmd+i);
        }
        // process
        if (strcmp(cmd+i, "SCALE=F") == 0)
            tflag = 'F';
        else if (strcmp(cmd+i, "SCALE=C") == 0)
            tflag = 'C';
        else if (strcmp(cmd+i, "STOP") == 0) //stop generating reports but continue processing input commands. If the program is not generating reports, merely log receipt of the command
            stop = 1;
        else if (strcmp(cmd+i, "START") == 0) //if stopped, resume generating reports. If the program is not stopped, merely log receipt of the command.
            stop = 0;
        else if (strcmp(cmd+i, "OFF") == 0) { //like pressing the button, output (and log) a time-stamped SHUTDOWN message and exit
            off = 1;
            shutdown();
        }
        else if (strlen(cmd) >= 3) { //change the number of seconds between reporting intervals. It is acceptable if this command does not take effect until after the next report
            char prefix_l[] = "LOG";
            int lg = 1;
            int k;
            for (k = 0; k < 3; k++) {
                if (cmd[i+k] != prefix_l[k])
                    lg = 0;
            }

            char prefix_p[] = "PERIOD="; 
            int prd = 1;
            if (lg == 0 && strlen(cmd) >= 7) {
                for (k = 0; k < 7; k++) 
                    if (cmd[i+k] != prefix_p[k])
                        prd = 0;
                if (prd == 1) {
                    int ret = stoi(cmd+i+7);
                    if (ret < 0)
                        fprintf(stderr, "Please provide a valid period\n");
                    else
                        sample_rate = ret;
                }
                else {
                    fprintf(stderr, "Please provide a valid command\n");
                    exit(1);
                }
            }

        }
        else {
            fprintf(stderr, "Please provide a valid command: %s\n", cmd+i);
            exit(1);
        }
        i = j+1;
    }


}



int main(int argc, char* argv[])
{
    /* declare variables */
    int opt = 0;

    /* set up polling */
    struct pollfd p[1];
    p[0].fd = 0;
    p[0].events = POLLIN | POLLHUP | POLLERR;   // data to read/hng up/error condition

    static struct option long_options[] = {
        { "period", required_argument, NULL, 'p' },
        { "scale", required_argument, NULL, 's' },
        { "log", required_argument, NULL, 'l' },
        { 0,0,0,0 }

    };
    while (1) {
        opt = getopt_long(argc, argv, "", long_options, NULL);
        if (opt == -1)
            break;
        switch(opt) {
            case 'p': {
                sample_rate = stoi(optarg);
                if (sample_rate < 0) {
                    fprintf(stderr, "Error: invalid period\n");
                    exit(1);
                }
                break;
            }
            case 's': {
                if (strcmp(optarg,"C") == 0)
                    tflag = 'C';
                break;
            }
            case 'l': {
                logfd = open(optarg, O_CREAT|O_WRONLY|O_APPEND, S_IRWXU);
                if (logfd == -1) {
                    fprintf(stderr, "%s\n", "Error: opening log file");
                    exit(1);
                }
                f_logfd = fopen(optarg, "a+");
                if (f_logfd == NULL) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            }
            default: {
                fprintf(stderr, "%s\n", "Error: invalid option");
                exit(1);
            }
        }
    }


    /* install signal handler */
    signal(SIGINT, sig_handler);

    /* initialize mraa aio and gpio */
    uint16_t value = 0;
    // mraa_aio_context aio;
    // mraa_gpio_context gpio;
    // aio = mraa_aio_init(AIO_PORT);
    // gpio = mraa_gpio_init(GPIO_PIN);
    // if (aio == NULL || gpio == NULL) {
    //     fprintf(stderr, "Error: mraa initialization\n");
    //     exit(1);
    // }
    // mraa_gpio_dir(gpio, MRAA_GPIO_IN);
    // mraa_gpio_isr(gpio, MRAA_GPIO_EDGE_RISING, (void*)&button_handler, NULL);
    

    /* get start time */
    struct timeval start_tv;
    if (gettimeofday(&start_tv, NULL) < 0) {
        fprintf(stderr, "Error: gettimeofday()\n");
        exit(1);
    }

    while (flag == 1) {

        /* get current time */ 
        struct timeval curr;
        if (gettimeofday(&curr, NULL) < 0) {
            fprintf(stderr, "Error: gettimeofday()\n");
            exit(1);
        }
        long count_down = curr.tv_sec - start_tv.tv_sec;

        /* sensor read */
        if (count_down >= sample_rate) {
            
            char timebuf[64];
            strftime(timebuf,64,"%T", localtime(&(curr.tv_sec)));
            

            /* read temperature */
            //value = mraa_aio_read(aio);
            value = 295;  // DUMMY
            float R = (1023.0/value - 1.0)*100000;
            float temp = 1.0/(log(R/100000.0)/4275.0+1/298.15)-273.15;
            if (tflag == 'F')
                temp = c_to_f(temp);

            /* report */
            if (stop == 0) {
                fprintf(stdout, "%s %.1f\n", timebuf, temp);
                if (logfd != -1)
                    fprintf(f_logfd, "%s %.1f\n", timebuf, temp);
            }

            start_tv = curr; // update current time
        }

        /* polling: read from terminal */
        int poll_status = poll(p, 1, 512);
        if (poll_status > 0) {  // 500 milliseconds, <1s
            char command[256];
            if ((p[0].revents&POLLIN) == POLLIN) {
                int r = read(0, command, 256);
                if (r < 0) {
                    fprintf(stderr, "Error: read()\n");
                }
                //else if (r == 0) continue;
                execute(command, r);
            }
        }

        if (off == 1)
            shutdown();
    }

    shutdown();

    close(logfd);
    // mraa_aio_close(aio);
    // mraa_gpio_close(gpio);

    return 0;
}