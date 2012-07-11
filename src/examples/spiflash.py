#!/usr/bin/env python

from mpsse import *

SIZE = 0x100000			# Size of SPI flash device: 1MB
RCMD = "\x03\x00\x00\x00"	# Standard SPI flash read command (0x03) followed by starting address (0x000000)
FOUT = "flash.bin"		# Output file

try:
	flash = MPSSE(SPI0, FIVE_MHZ, MSB)

	print "%s initialized at %dHz (SPI mode 0)" % (flash.GetDescription(), flash.GetClock())

	flash.Start()
	flash.Write(RCMD)
	data = flash.Read(SIZE)
	flash.Stop()

	open(FOUT, "wb").write(data)
	print "Dumped %d bytes to %s" % (len(data), FOUT)
	
	flash.Close()
except Exception, e:
	print "MPSSE failure:", e
	
