#!/usr/bin/env python
# Example code for reading from the AT93C46D microwire EEPROM chip.

import binascii
from mpsse import *

# Initialize the FTDI chip
at93c = MPSSE(SPI0, ONE_MHZ, MSB)

# Chip select idles low
at93c.SetCSIdle(0)

# Assert CS
at93c.Start()

# This assumes ORG pin is low (x8 mode)
# If ORG pin is high (x16 mode), there are only 6 address bits, not 7
#
# START BIT   READ COMMAND   ADDRESS BITS
# 1           1 0            0 0 0 0 0 0 0
at93c.WriteBits(1, 1)
at93c.WriteBits(1, 1)
at93c.WriteBits(0, 1)
at93c.WriteBits(0, 7)

# One dummy bit preceeds the actual data for each read operation
dummy = at93c.ReadBits(1)

# Read the whole chip
data = at93c.Read(128)

# Desassert CS
at93c.Stop()

# Close connection to the FTDI chip
at93c.Close()

print "Dummy bit:", dummy
print "Read %d bytes: %s" % (len(data), binascii.hexlify(data))
