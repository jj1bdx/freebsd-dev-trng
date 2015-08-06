/*
 * random_harvest injection device driver "trng"
 *
 * Based on
 * Example 9.1. Example of a Sample Echo Pseudo-Device Driver for FreeBSD 10.X,
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
 *
 */

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>

#include <sys/random.h>
#ifdef RNDTEST
#include <dev/rndtest/rndtest.h>
#endif /* RNDTEST */

/* Function prototypes */
static d_open_t      trng_open;
static d_close_t     trng_close;
static d_write_t     trng_write;

/* Character device entry points */
static struct cdevsw trng_cdevsw = {
    .d_version = D_VERSION,
    .d_open = trng_open,
    .d_close = trng_close,
    .d_write = trng_write,
    .d_name = "trng",
};

struct trng_softc {
    device_t device;
    struct cdev *cdev;
};

static devclass_t trng_devclass;

static void trng_identify(driver_t *driver, device_t parent)
{
    if (BUS_ADD_CHILD(parent, 33, "trng", 0) == NULL) {
        printf("trng: trng_identify failed\n");
    }
}

static int trng_probe(device_t dev)
{
    /* do nothing here */
    return (0);
}

static int trng_attach(device_t dev)
{
    struct trng_softc *sc = device_get_softc(dev);
    // int unit = device_get_unit(dev);
    int error = 0;

    sc->device = dev;
    error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
        &(sc->cdev), &trng_cdevsw, 0,
        UID_UUCP, GID_DIALER, 0660,
        "trng");
    if (error == 0) {
        sc->cdev->si_drv1 = sc;
    }
    return (error);
}

static int trng_detach(device_t dev)
{
    struct trng_softc *sc = device_get_softc(dev);

    destroy_dev(sc->cdev);
    return (0);
}

static device_method_t trng_methods[] = {
    DEVMETHOD(device_identify, trng_identify),
    DEVMETHOD(device_probe, trng_probe),
    DEVMETHOD(device_attach, trng_attach),
    DEVMETHOD(device_detach, trng_detach),
    DEVMETHOD_END
};

static driver_t trng_driver = {
    "trng",
    trng_methods,
    sizeof(struct trng_softc)
};

static int
trng_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
    int error = 0;
    /* really do nothing */
    return (error);
}

static int
trng_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
    struct thread *td __unused)
{
    /* really do nothing */
    return (0);
}

/* this buffer size is small */
/* see random_harvest(9) */
#define BUFFERSIZE (16)

/* maximum uio_resid size */
#define MAXUIOSIZE (1024)

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
    uint8_t buf[BUFFERSIZE];

#ifdef DEBUG
    printf("trng_write: uio->uio_resid: %zd\n", uio->uio_resid);
#endif /* DEBUG */
    /* check uio_resid size */
    if ((uio->uio_resid < 0) || (uio->uio_resid > MAXUIOSIZE)) {
#ifdef DEBUG
        printf("trng_write: invalid uio->uio_resid\n");
#endif /* DEBUG */
        return (EIO);
    }
    /* Copy the string to kernel memory */
    while (uio->uio_resid > 0) {
        amt = MIN(uio->uio_resid, BUFFERSIZE);
        error = uiomove(buf, amt, uio);
        if (error != 0) {
            /* error exit */
            return error;
        } 
        /* Enter the obtained data into random_harvest(9) */
        random_harvest(buf, amt,
            /* Caution: treated as a PURE random number sequence */
                    amt * NBBY / 2,
            /* TODO: must add a new class */
                    RANDOM_NET_ETHER);
#ifdef DEBUG
        printf("trng_write: put %zu bytes to random_harvest\n", amt);
#endif /* DEBUG */
    }
    /* normal exit */
    return 0;
}

/* TODO: is adding to "nexus" ok? */
DRIVER_MODULE(trng, nexus, trng_driver, trng_devclass, 0, 0);
