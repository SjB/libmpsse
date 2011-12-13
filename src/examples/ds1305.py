#!/usr/bin/env python

from mpsse import *
from time import sleep

if MPSSE(SPI1, ONE_HUNDRED_KHZ, MSB) == MPSSE_OK:

	# The DS1305's chip select pin idles low
	SetCSIdle(0)

	print "%s initialized at %dHz (SPI mode 1)" % (GetDescription(), GetClock())

	# Get the current control register value
	Start()
	Write("\x0F")
	control = ord(Read(1))
	Stop()

	# Clear the EOSC bit (BIT7) to enable the DS1305's oscillator
	control &= ~0x80

	# Write the new control value to the control register
	Start()
	Write("\x8F")
	Write(chr(control))
	Stop()

	# Loop to print the time elapsed every second
	while True:
	
		# Read in the elapsed seconds
		Start()
		Write("\x00")
		seconds = ord(Read(1))
		Stop()

		# High 4 bits == tens of seconds, low 4 bits == seconds
		seconds = (((seconds >> 4) * 10) + (seconds & 0x0F))

		# Read in the elapsed minutes
		Start()
		Write("\x01")
		minutes = ord(Read(1))
		Stop()	

		# High 4 bits == tens of minutes, low 4 bits == minutes
		minutes = (((minutes >> 4) * 10) + (minutes & 0x0F))

		print "%.2d:%.2d" % (minutes, seconds)

		sleep(1)
else:
	print "Failed to initialize MPSSE:", ErrorString()
	
Close()
