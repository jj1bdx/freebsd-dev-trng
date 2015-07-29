#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFFERSIZE 16

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE];
    int ttyfd, trngfd;
    ssize_t rsize, wsize;
    int errorcode;

    if ((ttyfd = open("/dev/cuaU0", O_RDONLY)) == -1) {
        perror("feedtrng: cannot open tty");
        exit(-1);
    }
    if ((trngfd = open("/dev/trng", O_WRONLY)) == -1) {
        perror("feedtrng: cannot open trng");
        exit(-1);
    }

    while(1) {
        if ((rsize = read(ttyfd, rbuf, (size_t)BUFFERSIZE)) == -1) {
            perror("feedtrng: tty read failed");
        }
        /* rewinding required for each writing */
        if ((wsize = pwrite(trngfd, rbuf, (size_t)rsize, (off_t)0)) == -1) {
            perror("feedtrng: trng write failed");
        }
        if (wsize < rsize) {
            fprintf(stderr,
                    "feedtrng: wsize %d < rsize %d, continues\n",
                    (int)wsize, (int)rsize);
        }
    }
    /* notreached */
    return 0;
}
    
