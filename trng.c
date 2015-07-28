/*
 * random_harvest injection device driver "trng"
 *
 * Based on the example at
 * Example 9.1. Example of a Sample Echo Pseudo-Device Driver for FreeBSD 10.X
 * Section 9.3. Character Devices,
 * Chapter 9. Writing FreeBSD Device Drivers,
 * FreeBSD Architecture Handbook, Revision 43184
 *
 * Original authors of
 * Simple Echo pseudo-device KLD:
 * 
 * Murray Stokely
 * Søren (Xride) Straarup
 * Eitan Adler
 * 
 * random_harvest injection code added by Kenji Rikitake
 *
 * Copyright (c) 2015 Kenji Rikitake
 * Copyright (c) 2000-2006, 2012-2013 The FreeBSD Documentation Project
 *
 * THIS DOCUMENTATION IS PROVIDED BY KENJI RIKITAKE AND THE FREEBSD
 * DOCUMENTATION PROJECT "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KENJI
 * RIKITAKE AND THE FREEBSD DOCUMENTATION PROJECT BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>  /* uprintf */
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/conf.h>   /* cdevsw struct */
#include <sys/uio.h>    /* uio struct */
#include <sys/malloc.h>
#include <sys/random.h>

/* this buffer size is small */
/* see random_harvest(9) */
#define BUFFERSIZE 16

/* Function prototypes */
static d_open_t      trng_open;
static d_close_t     trng_close;
static d_read_t      trng_read;
static d_write_t     trng_write;

/* Character device entry points */
static struct cdevsw trng_cdevsw = {
    .d_version = D_VERSION,
    .d_open = trng_open,
    .d_close = trng_close,
    .d_read = trng_read,
    .d_write = trng_write,
    .d_name = "trng",
};

struct s_trng {
    char msg[BUFFERSIZE + 1];
    int len;
};

/* vars */
static struct cdev *trng_dev;
static struct s_trng *trngmsg;

MALLOC_DECLARE(M_ECHOBUF);
MALLOC_DEFINE(M_ECHOBUF, "trngbuffer", "buffer for trng module");

/*
 * This function is called by the kld[un]load(2) system calls to
 * determine what actions to take when a module is loaded or unloaded.
 */
static int
trng_loader(struct module *m __unused, int what, void *arg __unused)
{
    int error = 0;

    switch (what) {
    case MOD_LOAD:                /* kldload */
        error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
            &trng_dev,
            &trng_cdevsw,
            0,
            UID_ROOT,
            GID_WHEEL,
            0600,
            "trng");
        if (error != 0)
            break;

        trngmsg = malloc(sizeof(*trngmsg), M_ECHOBUF, M_WAITOK |
            M_ZERO);
        printf("trng: device loaded.\n");
        break;
    case MOD_UNLOAD:
        destroy_dev(trng_dev);
        free(trngmsg, M_ECHOBUF);
        printf("trng: device unloaded.\n");
        break;
    default:
        error = EOPNOTSUPP;
        break;
    }
    return (error);
}

static int
trng_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
    int error = 0;
    /* really do nothing */
#ifdef DEBUG
    uprintf("Opened device \"trng\" successfully.\n");
#endif /* DEBUG */
    return (error);
}

static int
trng_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
    struct thread *td __unused)
{
    /* really do nothing */
#ifdef DEBUG
    uprintf("Closing device \"trng\".\n");
#endif /* DEBUG */
    return (0);
}

/*
 * The read function just takes the buf that was saved via
 * trng_write() and returns it to userland for accessing.
 * uio(9)
 */
static int
trng_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
    /* operation not supported by device */
    return (ENODEV);
}

/*
 * trng_write takes in a character string and
 * feeds the string to random_harvest(9),
 * as the pure random number sequence.
 */
static int
trng_write(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
    size_t amt;
    int error;

    /*
     * We either write from the beginning or are appending -- do
     * not allow random access.
     */
    if (uio->uio_offset != 0 && (uio->uio_offset != trngmsg->len)) {
        return (EINVAL);
    }
    /* This is a new message, reset length */
    if (uio->uio_offset == 0) {
        trngmsg->len = 0;
    }

    /* Copy the string in from user memory to kernel memory */
    amt = MIN(uio->uio_resid, (BUFFERSIZE - trngmsg->len));

    error = uiomove(trngmsg->msg + uio->uio_offset, amt, uio);

    /* Now we need to null terminate and record the length */
    trngmsg->len = uio->uio_offset;
    trngmsg->msg[trngmsg->len] = 0;

    if (error != 0) {
        uprintf("trng: write failed: bad address!\n");
    } else {
        /* Enter the obtained data into random_harvest(9) */
        random_harvest(trngmsg->msg, trngmsg->len,
        /* Caution: treated as a PURE random number sequence */
        (trngmsg->len) * NBBY / 2,
        /* TODO: must add a new class */
        RANDOM_NET_ETHER);
    }
    return (error);
}

DEV_MODULE(trng, trng_loader, NULL);
