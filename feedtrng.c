#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define BUFFERSIZE 16

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE];
    int trngfd;
    ssize_t rsize, wsize;
    FILE *fp_tty;
    int c, i;
    struct timespec tim;

    /* open TRNG tty and trng */
    if ((fp_tty = fopen("/dev/cuaU0", "r")) == NULL) {
        perror("feedtrng: cannot fopen tty");
        exit(-1);
    }
    /* open trng device */
    if ((trngfd = open("/dev/trng", O_WRONLY)) == -1) {
        perror("feedtrng: cannot open trng");
        exit(-1);
    }

    /* 10 milliseconds */
    tim.tv_sec = 0;
    tim.tv_nsec = 10 * 1000000L;

    while(1) {
        /* fill the receive buffer first */
        for(i = 0; i < BUFFERSIZE; i++) {
            c = EOF;
            /* wait until a valid character comes in */
            while (c == EOF) {
                /* wait 10msec before trying to get a character */
                if (nanosleep(&tim, NULL) == -1) {
                    perror("feedtrng: nanosleep failed");
                    exit(-1);
                }
                c = getc(fp_tty);
            }
            rbuf[i] = (uint8_t)c;
        }
        /* rewinding required for each writing */
        if ((wsize = pwrite(trngfd, rbuf,
                       (size_t)BUFFERSIZE,
                       (off_t)0)) == -1) {
            perror("feedtrng: trng write failed");
            exit(-1);
        }
    }
    /* notreached */
    return 0;
}
    
