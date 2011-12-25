import _mpsse

MPSSE_OK = _mpsse.MPSSE_OK
MPSSE_FAIL = _mpsse.MPSSE_FAIL
	
MSB = _mpsse.MSB
LSB = _mpsse.LSB

SPI0 = _mpsse.SPI0
SPI1 = _mpsse.SPI1
SPI2 = _mpsse.SPI2
SPI3 = _mpsse.SPI3
I2C = _mpsse.I2C

IFACE_ANY = _mpsse.IFACE_ANY
IFACE_A = _mpsse.IFACE_A
IFACE_B = _mpsse.IFACE_B
IFACE_C = _mpsse.IFACE_C
IFACE_D = _mpsse.IFACE_D

ONE_HUNDRED_KHZ = _mpsse.ONE_HUNDRED_KHZ
FOUR_HUNDRED_KHZ = _mpsse.FOUR_HUNDRED_KHZ
ONE_MHZ = _mpsse.ONE_MHZ
TWO_MHZ = _mpsse.TWO_MHZ
FIVE_MHZ = _mpsse.FIVE_MHZ
SIX_MHZ = _mpsse.SIX_MHZ
TEN_MHZ = _mpsse.TEN_MHZ
TWELVE_MHZ = _mpsse.TWELVE_MHZ
THIRY_MHZ = _mpsse.THIRTY_MHZ

class MPSSE:

	def __init__(self, mode=None, frequency=None, endianess=MSB):
		self.fd = MPSSE_FAIL
		if mode is not None and frequency is not None:
			self.fd = _mpsse.MPSSE(mode, frequency, endianess)
			if self.fd == MPSSE_FAIL:
				raise Exception, _mpsse.ErrorString(MPSSE_FAIL)

	def Open(self, vid, pid, interface, mode, frequency, endianess):
		self.fd = _mpsse.Open(vid, pid, interface, mode, frequency, endianess)
		if self.fd == MPSSE_FAIL:
			raise Exception, _mpsse.ErrorString(MPSSE_FAIL)
		return MPSSE_OK

	def Close(self):
		return _mpsse.Close(self.fd)
	
	def ErrorString(self):
		return _mpsse.ErrorString(self.fd)

	def SetMode(self, mode, endianess):
		return _mpsse.SetMode(self.fd, mode, endianess)

	def SetClock(self, frequency):
		return _mpsse.SetClock(self.fd, frequency)

	def GetClock(self):
		return _mpsse.GetClock(self.fd)

	def GetVid(self):
		return _mpsse.GetVid(self.fd)

	def GetPid(self):
		return _mpsse.GetPid(self.fd)

	def GetDescription(self):
		return _mpsse.GetDescription(self.fd)

	def SetLoopback(self, enable):
		return _mpsse.SetLoopback(self.fd, enable)

	def SetCSIdle(self, idle):
		return _mpsse.SetCSIdle(self.fd, idle)

	def Start(self):
		return _mpsse.Start(self.fd)

	def Stop(self):
		return _mpsse.Stop(self.fd)

	def Write(self, data):
		return _mpsse.Write(self.fd, data)

	def Read(self, size):
		return _mpsse.Read(self.fd, size)

	def SetAck(self, ack):
		return _mpsse.SetAck(self.fd, ack)

	def GetAck(self):
		return _mpsse.GetAck(self.fd)

	def PinHigh(self, pin):
		return _mpsse.PinHigh(self.fd, pin)

	def PinLow(self, pin):
		return _mpsse.PinLow(self.fd, pin)

