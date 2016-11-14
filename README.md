# /dev/trng: an entropy injection device driver for FreeBSD

## WARNING

This software is still at the *experimental stage*. No guarantee for any damage
which might be caused by the use of this software. Caveat emptor.

## IMPORTANT SECURITY NOTICE

*Note well: providing incorrect permission and unauthenticated or unscreened
data to the device driver /dev/trng may degrade the quality of /dev/random,
/dev/urandom, and the security of the entire FreeBSD operating system.*

## What this driver is for

To accomodate True Random Number Generator (TRNG) random bits into FreeBSD
kernel.

*Note: this driver is for FreeBSD 11.0 and later only.*

## Tested environment

* FreeBSD amd64 11.0-STABLE r308609

## How this works

The driver in `trng.c` works as `/dev/trng`. It accepts up to 1024-byte write
operation to feed the written data as an entropy string by calling random\_harvest\_fast(9) multiple times. 16 bytes in maximum are passed for each time when the harvesting function is called. 

`feedtrng.c` is a C code example to transfer TRNG data from a tty device to
`/dev/trng`. The code sets input tty disciplines and lock the tty, then feed
the contents to `/dev/trng`. Some things to consider:

* The tty input are sampled as 512-byte blocks. Each block is concatenated to
64-byte hashed result of SHA512, and again hashed by SHA512, to obtain
64-byte (512-bit) hashed output. The hashed result is sent to the kernel.
Compression ratio: 1/8.  This whitening can be disabled by `-t` option.
* When running in the default mode, the first block (512 bytes) from the tty device is *discarded* to prevent unstable data of TRNG from being transferred to `/dev/trng`. This data *truncation does not happen* when the data is redirected to
stdout.

The source to write to `/dev/trng` *must* be a real TRNG. Possible candidates are:

* [avrhwrng](https://github.com/jj1bdx/avrhwrng/), an experimental hardware on Arduino Duemilanove/UNO, claiming ~10kbytes/sec (disclaimer: Kenji Rikitake develops the software and hardware)
* [NeuG](http://www.gniibe.org/memo/development/gnuk/rng/neug.html), claiming ~80kbytes/sec generation speed
* [OneRNG](http://onerng.info), claiming ~44kbytes/sec generation speed
* [TrueRNG 2](https://www.tindie.com/products/ubldit/truerng-hardware-random-number-generator/), claiming ~43.5kbytes/sec generation speed

Currently, the random bits in the given entropy strings are estimated as 1/2 of
the given bits, which is a common practice for accepting TRNG sequences in the
FreeBSD crypto device drivers.

The entropy source provided by `/dev/trng` is currently indicated as
`RANDOM_NET_ETHER`. Set `sysctl kern.random.sys.harvest.ethernet=1` to enable
harvesting from the Ethernet traffic. See random(4) for the details. 

## Version

* 14-NOV-2016: 0.5.0 (FreeBSD 11 or later only, remove rndtest(4) support)
* 10-JUL-2016: 0.4.0 (Preliminary fix for FreeBSD 11)
* 10-OCT-2015: 0.3.3 (Use SHA512 hash for 1:8 compression as default)
* 7-OCT-2015: 0.3.2 (Make feedtrng to discard the first block from tty)
* 23-SEP-2015: 0.3.1 (Fix termios; now CLOCAL cleared, modem control enabled)
* 20-SEP-2015: 0.3.0 (Installation simplified, Makefiles streamlined)
* 19-SEP-2015: 0.2.3 (Fix feedtrng tty read(2) bug)
* 13-AUG-2015: 0.2.2 (Add feedtrng `-o` option for redirecting output to stdout)
* 12-AUG-2015: 0.2.1 (Fix feedtrng tcsetattr bug)
* 12-AUG-2015: 0.2.0 (Revise feedtrng to set tty line disciplines, exclusive access)
* 11-AUG-2015: 0.1.1 (Revise feedtrng to accept `/dev/cua*` device name)
*  6-AUG-2015: 0.1.0 (Use Newbus driver, enable rndtest driver hook)
*  4-AUG-2015: 0.0.5 (Fix trng dev code)
* 30-JUL-2015: 0.0.4 (Fix on code)
* 29-JUL-2015: 0.0.3 (Fix feedtrng code)
* 28-JUL-2015: 0.0.1 (Initial release)

## How to compile and load /dev/trng

    make clean all
    # run following as a superuser
    # trng.ko will be added to /boot/modules/
    # feedtrng will be added to /usr/local/bin/
    make install
    # /dev/trng has the owner uucp:dialer and permission 0660 as default
    kldload trng.ko

## How to run feedtrng

    # Only /dev/cua* devices are accepted
    feedtrng -d /dev/cuaU0
    # only the basename(3) part is used and attached to `/dev/` directly
    # so this is also OK
    feedtrng -d cuaU0
    # tty speed [bps] can be set (9600 ~ 1000000, default 115200)
    feedtrng -d cuaU1 -s 9600
    # for usage
    feedtrng -h

## tty discipline of the input tty

    # result of `sudo stty -f /dev/cuaU0` (sudo needed to override TIOCEXCL)
    speed 115200 baud;
    lflags: -icanon -isig -iexten -echo
    iflags: -icrnl -ixon -imaxbel ignbrk -brkint
    oflags: -opost tab0
    cflags: cs8 -parenb -clocal

## License

BSD 2-clause. See LICENSE.

SHA512 hashing code are from the following page: [Fast SHA-2 hashes in x86 assembly](http://www.nayuki.io/page/fast-sha2-hashes-in-x86-assembly) by Project Nayuki. The related code are distributed under the MIT License.

## rndtest(4) functions and the usage

This part is no longer applicable for `/dev/trng`. See `rndtest.md` for the further details.

## References

* [FreeBSD Architecture Handbook](https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/index.html): Section 9.3 <https://www.freebsd.org/doc/en_US.ISO8859-1/books/arch-handbook/driverbasics-char.html>
* [Writing a kernel module for FreeBSD](http://www.freesoftwaremagazine.com/articles/writing_a_kernel_module_for_freebsd)
* Joseph Kong, [FreeBSD Device Drivers](https://www.nostarch.com/bsddrivers.htm), No Starch Press, May 2012, ISBN 978-1-59327-204-3
