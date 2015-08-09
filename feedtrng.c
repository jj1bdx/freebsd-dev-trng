/*
 * Feeder for /dev/trng
 * by Kenji Rikitake
 *
 * Copyright (c) 2015 Kenji Rikitake
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <errno.h>

#define CMDNAME "feedtrng"

void usage(void) {
    fprintf(stderr, "Usage: %s -d pathname\n", CMDNAME);
    fprintf(stderr, "(Only the basename part is used)\n");
    fprintf(stderr, "(The device name = /dev/[basename])\n");
    fprintf(stderr, "Usage: %s -h for help\n", CMDNAME);
    exit (-1);
}

void printerror(const char *p) {

    fprintf(stderr, "%s: %s", CMDNAME, p);
    if (errno > 0) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    putc('\n', stderr);
}

/* 
 * buffer size
 * for fetching from the TRNG tty device
 * designed for NeuG (~80kbytes/sec)
 * change the value for a higher-speed device
 */

#define BUFFERSIZE (512)

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE], *p;
    int ttyfd, trngfd;
    ssize_t rsize, wsize;
    int i;
    int dflag = 0;
    int ch;
    char *input;
    char inputbase[MAXPATHLEN];
    char devname[MAXPATHLEN];

    if (argc < 2) {
        usage();
    }
    while ((ch = getopt(argc, argv, "d:h")) != -1) {
        switch (ch) {
        case 'd':
            dflag = 1;
            if ((input = strndup(optarg, MAXPATHLEN)) == NULL) {
                printerror("device input string error");
                exit(-1);
            }
            if (NULL == basename_r(input, inputbase)) {
                printerror("device input basename_r failed");
                exit(-1);
            }
            if ((*inputbase == '/') || (*inputbase == '.')) {
                printerror("illegal path in inputbase");
                exit(-1);
            }
            break;
        case 'h':
            usage();
            break;
        case '?':
        default:
            usage();
        }
    }
    if (dflag == 0) {
        printerror("no device name given");
        exit(-1);
    }
    if ((strlcpy(devname, "/dev/", MAXPATHLEN)) >= MAXPATHLEN) {
        printerror("strlcpy devname failed");
        exit(-1);
    }
    if ((strlcat(devname, inputbase, MAXPATHLEN)) >= MAXPATHLEN) {
        printerror("strlcat devname failed");
        exit(-1);
    }
#ifdef DEBUG
    fprintf(stderr,
        "feedtrng: device name: %s\n",
        devname);
#endif
    /* open TRNG tty */
    if ((ttyfd = open(devname, O_RDONLY)) == -1) {
        printerror("cannot open tty");
        exit(-1);
    }
    /* open trng device */
    if ((trngfd = open("/dev/trng", O_WRONLY)) == -1) {
        printerror("cannot open trng");
        exit(-1);
    }

    /* infinite loop */
    while (1) {
        /* fill the receive buffer first */
        for (i = 0, p = rbuf; i < BUFFERSIZE; ) {
            /* try reading from tty */
            if ((rsize = read(ttyfd, p + i,
                            BUFFERSIZE - i)) == -1) {
                printerror("read from tty failed");
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
            printerror("trng write failed");
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
