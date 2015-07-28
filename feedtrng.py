#!/usr/bin/env python

import os, serial

if __name__ == '__main__':
    ser = serial.Serial(
            port='/dev/cuaU0',
            baudrate=19200,
            timeout=10,
            xonxoff=0,
            rtscts=0)
    # ser.open()
    fdtrng = os.open('/dev/trng', os.O_RDWR)
    # discard first 128 bytes
    dummy = ser.read(128)
    # receive loop
    # keep bytes small
    while 1:
	s = ser.read(16)
	os.write(fdtrng, s)
