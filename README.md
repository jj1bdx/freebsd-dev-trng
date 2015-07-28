# An entropy injection device driver for FreeBSD

## WARNING

This software is still at the *experimental stage*. No guarantee for any damage
which might be caused by the use of this software. Caveat emptor.

## What this driver is for

To accept True Random Number Generator (TRNG) random bits into FreeBSD kernel.

## IMPORTANT SECURITY NOTICE

*Note well: providing incorrect permission to the device loaded by this driver
may degrade the quality of /dev/random, /dev/urandom, and the security of the
entire FreeBSD operating system.*

## How this works

This driver works as `/dev/trng`, and accepts up to 16-byte write operation to
feed it as an entropy string to random_harvest(9). Use write(2) for writing to
the device. `feedtrng.py` is a Python code example.

The source to write to `/dev/trng` *must* be a real TRNG. Possible candidates are:

* [NeuG](http://www.gniibe.org/memo/development/gnuk/rng/neug.html), claiming ~80kbytes/sec generation speed
* [TrueRNG 2](https://www.tindie.com/products/ubldit/truerng-hardware-random-number-generator/), claiming ~43.5kbytes/sec generation speed

The following TRNG is slow (~2kbytes/sec), but may work well (disclaimer: Kenji
Rikitake develops the software and hardware):

* [avrhwrng](https://github.com/jj1bdx/avrhwrng/), an experimental hardware on Arduino Duemilanove/UNO

Currently, the random bits in the given entropy strings are estimated as 1/2 of
the given bits, which is a common practice for accepting TRNG sequences.

The entropy source is currently indicated as `RANDOM_NET_ETHER`. Set `sysctl
kern.random.sys.harvest.ethernet=1` to enable harvesting from the Ethernet
traffic. See random(4) for the details. *TODO: an entirely new class should be
introduced.*

## How to compile and load

    make
    # run following as a superuser
    kldload ./trng.ko
    # give necessary access permission to /dev/trng
    chown uucp:dialer /dev/trng
    chmod 660 /dev/trng

## License

See `trng.c`, the same as FreeBSD Architecture Handbook.

## References

* [FreeBSD Architecture Handbook](https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/index.html): Section 9.3 <https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/driverbasics-char.html>
* [Writing a kernel module for FreeBSD](http://www.freesoftwaremagazine.com/articles/writing_a_kernel_module_for_freebsd)
