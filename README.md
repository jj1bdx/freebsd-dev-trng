# /dev/trng: an entropy injection device driver for FreeBSD

## WARNING

This software is still at the *experimental stage*. No guarantee for any damage
which might be caused by the use of this software. Caveat emptor.

## What this driver is for

To accomodate True Random Number Generator (TRNG) random bits into FreeBSD
kernel.

## IMPORTANT SECURITY NOTICE

*Note well: providing incorrect permission and unauthenticated or unscreened
data to the device driver /dev/trng may degrade the quality of /dev/random,
/dev/urandom, and the security of the entire FreeBSD operating system.*

## Tested environment

* FreeBSD 10.2-PRERELEASE amd64 r286356
* Enabling rndtest(4) strongly recommended

## How this works

The driver in `trng.c` works as `/dev/trng`, and accepts up to 1024-byte write
operation to feed the written data as an entropy string by calling
random\_harvest(9) multiple times. 16 bytes are passed for each time when
random\_harvest(9) is called. 

`feedtrng.c` is a C code example to transfer TRNG data from a tty device to
`/dev/trng`.

The source to write to `/dev/trng` *must* be a real TRNG. Possible candidates are:

* [NeuG](http://www.gniibe.org/memo/development/gnuk/rng/neug.html), claiming ~80kbytes/sec generation speed
* [TrueRNG 2](https://www.tindie.com/products/ubldit/truerng-hardware-random-number-generator/), claiming ~43.5kbytes/sec generation speed

The following TRNG is slow (~2kbytes/sec), but may work well (disclaimer: Kenji
Rikitake develops the software and hardware):

* [avrhwrng](https://github.com/jj1bdx/avrhwrng/), an experimental hardware on Arduino Duemilanove/UNO

Currently, the random bits in the given entropy strings are estimated as 1/2 of
the given bits, which is a common practice for accepting TRNG sequences in the
FreeBSD crypto device drivers.

The entropy source is currently indicated as `RANDOM_NET_ETHER`. Set `sysctl
kern.random.sys.harvest.ethernet=1` to enable harvesting from the Ethernet
traffic. See random(4) for the details. *TODO: an entirely new class should be
introduced.*

## Version

* 11-AUG-2015: 0.1.1 (feedtrng revised to accept `/dev/cua*` device name)
* 6-AUG-2015: 0.1.0 (Use Newbus driver, enable rndtest driver hook)
* 4-AUG-2015: 0.0.5 (fix trng dev code)
* 30-JUL-2015: 0.0.4 (fix on code)
* 29-JUL-2015: 0.0.3 (fix feedtrng code)
* 28-JUL-2015: 0.0.1 (initial release)

## How to compile and load /dev/trng

    make clean all
    # run following as a superuser
    # /dev/trng has the owner uucp:dialer and permission 0660 as default
    kldload ./trng.ko

## How to run feedtrng

    # Only /dev/cua* devices are accepted
    feedtrng -d /dev/cuaU0
    # only the basename(3) part is used and attached to `/dev/` directly
    # so this is also OK
    feedtrng -d cuaU0

## stty settings required for raw mode

    # result of `stty -f /dev/cuaU0`
    # (use `stty -f /dev/cuaU0.init speed 115200 clocal raw` to configure)
    speed 115200 baud;
    lflags: -icanon -isig -iexten -echo
    iflags: -icrnl -ixon -imaxbel ignbrk -brkint
    oflags: -opost tab0
    cflags: cs8 -parenb clocal

## License

BSD 2-clause. See LICENSE.

## References

* [FreeBSD Architecture Handbook](https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/index.html): Section 9.3 <https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/driverbasics-char.html>
* [Writing a kernel module for FreeBSD](http://www.freesoftwaremagazine.com/articles/writing_a_kernel_module_for_freebsd)
