#!/usr/bin/env python

from mpsse import *

FREQ = 30000000			# SPI clock frequency: 30MHz
SIZE = 0x100000			# Size of SPI flash device: 1MB
WCMD = "\x03\x00\x00\x00"	# Standard SPI flash read command (0x03) followed by starting address (0x000000)
FILE = "spi.dump"

if MPSSE(SPI3, FREQ, MSB) == MPSSE_OK:

	print "Reading %d bytes from SPI flash chip..." % SIZE

	Start()
	Write(WCMD)
	data = Read(SIZE)
	Stop()

	Close()

	open(FILE, "wb").write(data)
	print "Dumped %d bytes to %s" % (len(data), FILE)
else:
	print "Failed to initialize MPSSE:", ErrorString()
