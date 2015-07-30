#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define BUFFERSIZE (512)

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE], *p;
    int ttyfd, trngfd;
    ssize_t rsize, wsize;
    int i;

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

    /* infinite loop */
    while (1) {
        /* fill the receive buffer first */
        for (i = 0, p = rbuf; i < BUFFERSIZE; ) {
            /* try reading from tty */
            if ((rsize = read(ttyfd, p + i,
                            BUFFERSIZE - i)) == -1) {
                perror("feedtrng: read from tty failed");
                exit(-1);
            }
#ifdef DEBUG
            fprintf(stderr,
                    "feedtrng: rsize %d after read\n",
                    (int)rsize);
#endif
            /* add the number of bytes read */
            i += rsize;
#ifdef DEBUG
            fprintf(stderr,
                    "feedtrng: i %d after read\n",
                    (int)i);
#endif
        }
        if ((wsize = write(trngfd, rbuf,
                       (size_t)BUFFERSIZE)) == -1) {
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
    
