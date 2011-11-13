#!/usr/bin/env python

from mpsse import *

SIZE = 0x8000		# Size of EEPROM chip (32 KB)
WCMD = "\xA0\x00\x00"	# Write start address command
RCMD = "\xA1"		# Read command
FOUT = "eeprom.bin"	# Output file

if MPSSE(I2C, FOUR_HUNDRED_KHZ, MSB) == MPSSE_OK:

	print "%s initialized at %dHz (I2C)" % (GetDescription(), GetClock())

	Start()
	Write(WCMD)

	if GetAck() == 0:

		Start()
		Write(RCMD)
	
		if GetAck() == 0:
			data = Read(SIZE)

		Stop()
	
	open(FOUT, "wb").write(data)	
	print "Dumped %d bytes to %s" % (len(data), FOUT)
else:
	print "Failed to initialize MPSSE:", ErrorString()
	
Close()
