#!/usr/bin/env python

from mpsse import *

MPSSE(SPI3, 30000000, MSB)

SetTimeouts(20)

Start()
Write("\x03\x00\x00\x00")
data = Read(0x100000)
Stop()

Close()

print "Got %d bytes of data!" % len(data)
open("data.dump", "wb").write(data)
