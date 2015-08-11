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
#include <err.h>
#include <sysexits.h>
#include <termios.h>
#include <limits.h>

void usage(void) {
    errx(EX_USAGE,
        "Usage: %s [-d cua-device] [-s speed] [-h]\n"
        "Only cua[.+] and /dev/cua[.+] are accepted\n"
        "Speed range: 9600 to 1000000 [bps] (default: 115200)\n"
        "Use -h for help", getprogname());
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
    struct termios ttyconfig;
    ssize_t rsize, wsize;
    int i;
    int dflag = 0;
    int ch;
    char *input;
    char inputbase[MAXPATHLEN];
    char devname[MAXPATHLEN];
    long speedval = 115200L;

    if (argc < 2) {
        usage();
    }
    while ((ch = getopt(argc, argv, "d:s:h")) != -1) {
        switch (ch) {
        case 'd':
            dflag = 1;
            if ((input = strndup(optarg, MAXPATHLEN)) == NULL) {
                errx(EX_USAGE, "device input string error");
            }
            if (NULL == basename_r(input, inputbase)) {
                errx(EX_OSERR, "device input basename_r failed");
            }
            if ((*inputbase == '/') || (*inputbase == '.')) {
                errx(EX_USAGE, "illegal path in inputbase");
            }
            break;
        case 's':
            errno = 0;
            speedval = strtol(optarg, NULL, 10);
            if (errno > 0) {
                err(EX_OSERR, "strtol for speedval failed");
            }
            if ((speedval < 9600) || (speedval > 1000000)) {
                errx(EX_USAGE, "speedval %ld out of range", speedval);
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
        errx(EX_USAGE, "no device name given");
    }
    if (strnlen(inputbase, 4) < 4) {
        errx(EX_USAGE, "input basename less than four letters");
    }
    if ((inputbase[0] != 'c') ||
        (inputbase[1] != 'u') ||
        (inputbase[2] != 'a')) {
        errx(EX_USAGE, "not a /dev/cua* device");
    }
    if ((strlcpy(devname, "/dev/", MAXPATHLEN)) >= MAXPATHLEN) {
        errx(EX_OSERR, "strlcpy devname failed");
    }
    if ((strlcat(devname, inputbase, MAXPATHLEN)) >= MAXPATHLEN) {
        errx(EX_OSERR, "strlcat devname failed");
    }
#ifdef DEBUG
    fprintf(stderr,
        "feedtrng: device name: %s\n",
        devname);
#endif
    /* open TRNG tty */
    if ((ttyfd = open(devname, O_RDONLY)) == -1) {
        err(EX_IOERR, "cannot open tty file");
    }
    /* check if really a tty */
    if (0 == isatty(ttyfd)) {
        err(EX_IOERR, "input not a tty");
    }
    /* set clocal mode (no modem) */
    if (-1 == tcgetattr(ttyfd, &ttyconfig)) {
        err(EX_IOERR, "input tcgetattr failed");
    }
    if (-1 == cfsetspeed(&ttyconfig, B0)) {
        err(EX_IOERR, "input cfsetspeed to B0 failed");
    }
    if (-1 == tcsetattr(ttyfd, TCSANOW, &ttyconfig)) {
        err(EX_IOERR, "input tcsetattr for clocal failed");
    }
    /* set RAW mode */
    cfmakeraw(&ttyconfig);
    /* set speed */
    if (-1 == cfsetspeed(&ttyconfig, (speed_t)speedval)) {
        err(EX_IOERR, "input cfsetspeed to %ld failed", speedval);
    }
    if (-1 == tcsetattr(ttyfd, TCSANOW, &ttyconfig)) {
        err(EX_IOERR,
            "input tcsetattr for raw and speed %ld failed", speedval);
    }

    /* open trng device */
    if ((trngfd = open("/dev/trng", O_WRONLY)) == -1) {
        err(EX_IOERR, "cannot open /dev/trng");
    }

    /* infinite loop */
    while (1) {
        /* fill the receive buffer first */
        for (i = 0, p = rbuf; i < BUFFERSIZE; ) {
            /* try reading from tty */
            if ((rsize = read(ttyfd, p + i,
                            BUFFERSIZE - i)) == -1) {
                err(EX_IOERR, "read from tty failed");
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
            err(EX_IOERR, "trng write failed");
        }
#ifdef DEBUG
        fprintf(stderr, "feedtrng: write %d bytes\n",
                (int)wsize);
#endif
    }
    /* notreached */
    return 0;
}
