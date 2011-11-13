#!/usr/bin/env python

from mpsse import *

SIZE = 0x100000			# Size of SPI flash device: 1MB
RCMD = "\x03\x00\x00\x00"	# Standard SPI flash read command (0x03) followed by starting address (0x000000)
FOUT = "flash.bin"		# Output file

if MPSSE(SPI0, THIRTY_MHZ, MSB) == MPSSE_OK:

	print "%s initialized at %dHz (SPI mode 0)" % (GetDescription(), GetClock())

	Start()
	Write(RCMD)
	data = Read(SIZE)
	Stop()

	open(FOUT, "wb").write(data)
	print "Dumped %d bytes to %s" % (len(data), FOUT)
else:
	print "Failed to initialize MPSSE:", ErrorString()
	
Close()
