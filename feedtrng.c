#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define BUFFERSIZE 16

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE];
    int ttyfd, trngfd;
    ssize_t rsize, wsize;
    int i;
    struct timespec tim;

    /* open TRNG tty */
    if ((ttyfd = open("/dev/cuaU0", O_RDONLY)) == -1) {
        perror("feedtrng: cannot open tty");
        exit(-1);
    }
    /* open trng device */
    if ((trngfd = open("/dev/trng", O_WRONLY)) == -1) {
        perror("feedtrng: cannot open trng");
        exit(-1);
    }

    /* 1000 microseconds */
    tim.tv_sec = 0;
    tim.tv_nsec = 1000 * 1000L;

    /* infinite loop */
    while (1) {
        /* fill the receive buffer first */
        for (i = 0; i < BUFFERSIZE; ) {
            /* try reading from tty */
            if ((rsize = read(ttyfd, &rbuf + i,
                            BUFFERSIZE - i)) == -1) {
                perror("feedtrng: read from tty failed");
                exit(-1);
            }
#ifdef DEBUG
            fprintf(stderr,
                    "feedtrng: rsize %d after read\n",
                    (int)rsize);
#endif
            i += rsize;
#ifdef DEBUG
            fprintf(stderr,
                    "feedtrng: i %d after read\n",
                    (int)i);
#endif
            /* when rsize = 0, wait and retry */
            if (rsize == 0) {
                /* wait before trying to get a character */
                if (nanosleep(&tim, NULL) == -1) {
                    perror("feedtrng: nanosleep failed");
                    exit(-1);
                }
            }
        }
        /* rewinding required for each writing */
        if ((wsize = pwrite(trngfd, rbuf,
                       (size_t)BUFFERSIZE,
                       (off_t)0)) == -1) {
            perror("feedtrng: trng write failed");
            exit(-1);
        }
#ifdef DEBUG
        fprintf(stderr, "feedtrng: write %d bytes\n",
                (int)wsize);
#endif
    }
    /* notreached */
    return 0;
}
    
