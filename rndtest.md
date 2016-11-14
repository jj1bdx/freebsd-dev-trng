# rndtest(4) functions and the usage

rndtest(4) is a pseudo device driver for accepting True Random Number
Generator (TRNG) *after* testing the stochastic property of the RNG data
stream.  The data passed the test are handed over to random_harvest(9)
with the class `RANDOM_PURE_RNDTEST`.

rndtest(4) provides the following three functions in the header file `<dev/rndtent/rndtest.h>`:

* `rndtest_attach()`: allocating a set of rndtest(4) resource for a given
  device of Newbus.
* `rndtest_detach()`: deallocating a set of resource assigned by
  `rndtest_attach()`.
* `rndtest_harvest()`: Accepting a data string of given length to the
  stochactic property test, then hand over the string which passes the
  test to `random_harvest(9)`.

Usage examples of rndtest(4) functions are found in the source code of
security hardware device drivers, such as hifn(4) and safe(4). Here are
some things to consider:

* The parent device *must* be a Newbus driver since rndtest(4) uses the
  parent driver's notification mechanism. For a "real" I/O driver this
  won't be a big problem, but for a "pseudo" driver, the programmer must
  decide which bus to attach.  For trng, `nexus` is the current choice,
  because it doesn't have any physical device to initialize.

* `BUS_ADD_CHILD()` *must be run only once* when running the
  `device_identify` method. If the same name of child found by
  `device_find_child()`, the device is already added, so do not add a
  new one. Mishandling this may cause the system crash and reboot.

* `rndtest_attach()` may fail, and in case of the failure,
  `random_harvest()` must be used instead of `rndtest_harvest()`. Using
  a function pointer, which is found in safe(4), hifn(4), and also in
  trng, will make this process easier.

## rndtest(4) sysctl OIDs and values

* `kern.rndtest.retest`: retest interval in seconds
* `kern.rndtest.verbose`: report to console (test failures only)
* `kern.rndtest.stats`: rndtest stats of opaque binary values (six `uint32_t` values)

A one-liner shell script to decode `kern.rndtest.stats` (see the details in
`<dev/rndtest/rndtest.h>`):

```
sysctl -b kern.rndtest.stats | \
hexdump -e '6/4 "%12d" "\n"' | \
awk '{print "rst_discard: " $1; \
      print "rst_tests: " $2; \
      print "rst_monobit: " $3; \
      print "rst_runs: " $4; \
      print "rst_longruns: " $5; \
      print "rst_chi: " $6;} '
```
