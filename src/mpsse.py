import pylibmpsse as _mpsse

MPSSE_OK = _mpsse.MPSSE_OK
MPSSE_FAIL = _mpsse.MPSSE_FAIL
	
MSB = _mpsse.MSB
LSB = _mpsse.LSB

ACK = _mpsse.ACK
NACK = _mpsse.NACK

SPI0 = _mpsse.SPI0
SPI1 = _mpsse.SPI1
SPI2 = _mpsse.SPI2
SPI3 = _mpsse.SPI3
I2C = _mpsse.I2C
GPIO = _mpsse.GPIO
BITBANG = _mpsse.BITBANG

GPIOL0 = _mpsse.GPIOL0
GPIOL1 = _mpsse.GPIOL1
GPIOL2 = _mpsse.GPIOL2
GPIOL3 = _mpsse.GPIOL3
GPIOH0 = _mpsse.GPIOH0
GPIOH1 = _mpsse.GPIOH1
GPIOH2 = _mpsse.GPIOH2
GPIOH3 = _mpsse.GPIOH3
GPIOH4 = _mpsse.GPIOH4
GPIOH5 = _mpsse.GPIOH5
GPIOH6 = _mpsse.GPIOH6
GPIOH7 = _mpsse.GPIOH7

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
THIRTY_MHZ = _mpsse.THIRTY_MHZ

class MPSSE:
	"""
	Python class wrapper for libmpsse.
	"""

	def __init__(self, mode=None, frequency=ONE_MHZ, endianess=MSB):
		"""
		If mode is specified, then attempt to connect to any known FTDI chip.
		If mode is not specified, this simply returns the class instance.
		Endianess defaults to MSB.
		"""
		self.context = None
		if mode is not None:
			self.context = _mpsse.mpsse_easy_open(mode, frequency, endianess)
			if self.context.open == 0:
				raise Exception, self.ErrorString()

	def Open(self, vid, pid, mode, frequency, endianess=MSB, interface=IFACE_A, description=None, serial=None):
		"""
		Opens the specified USB device.
		Endianess defaults to MSB; interface defaults to IFACE_A; description and serial default to None.
		"""
		self.context = _mpsse.mpsse_open(vid, pid, mode, frequency, endianess, interface, description, serial)
		if self.context.open == 0:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def Close(self):
		"""
		Closes the device, deinitializes libftdi, and frees the libmpsse context.
		"""
		retval = _mpsse.mpsse_close(self.context)
		self.context = None
		return retval
	
	def ErrorString(self):
		"""
		Returns the last libftdi error string.
		"""
		return _mpsse.mpsse_error_string(self.context)

	def SetMode(self, mode, endianess):
		"""
		Sets the appropriate transmit and receive commands based on the requested mode and byte order.
		Called internally by __init__ and Open.
		"""
		if _mpsse.mpsse_set_mode(self.context, mode, endianess) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def SetClock(self, frequency):
		"""
		Sets the appropriate divisor for the desired clock frequency. Frequency must be specified in hertz.
		Called internally by __init__ and Open.
		"""
		if _mpsse.mpsse_set_clock(self.context, frequency) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def GetClock(self):
		"""
		Gets the currently configured clock rate.
		"""
		return _mpsse.mpsse_get_clock(self.context)

	def GetVid(self):
		"""
		Returns the vendor ID of the FTDI chip.
		"""
		return _mpsse.mpsse_get_vid(self.context)

	def GetPid(self):
		"""
		Returns the product ID of the FTDI chip.
		"""
		return _mpsse.mpsse_get_pid(self.context)

	def GetDescription(self):
		"""
		Returns the description of the FTDI chip, if any. 
		This will only be populated if __init__ is used to open the device.
		"""
		return _mpsse.mpsse_get_description(self.context)

	def SetLoopback(self, enable):
		"""
		Enable / disable internal loopback. Loopback is disabled by default.
		Set enable = 1 to enable, enable = 0 to disable.
		"""
		if _mpsse.mpsse_set_loopback(self.context, enable) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def SetCSIdle(self, idle):
		"""
		Sets the idle state of the chip select pin. CS idles high by default.
		Only appropriate when using one of the SPI modes.
		Set idle = 1 to idle high, idle = 0 to idle low.
		"""
		return _mpsse.mpsse_set_cs_idle(self.context, idle)

	def Start(self):
		"""
		Send data start condition.
		"""
		if _mpsse.mpsse_start(self.context) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def Stop(self):
		"""
		Send data stop condition.
		"""
		if _mpsse.mpsse_stop(self.context) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def Write(self, data):
		"""
		Send data (string) out via the selected serial protocol.
		"""
		if _mpsse.mpsse_write(self.context, data) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def Read(self, size):
		"""
		Reads size bytes of data over the selected serial protocol.
		"""
		return _mpsse.mpsse_read(self.context, size)

	def Transfer(self, data):
		"""
		Sends data and reads len(data) bytes in SPI mode.
		"""
		return _mpsse.mpsse_transfer(self.context, data)

	def SetAck(self, ack):
		"""
		Sets the transmitted ACK bit. ACKs are sent by default.
		Set ack = 1 to send ACKs, ack = 0 to send NACKs.
		"""
		return _mpsse.mpsse_set_ack(self.context, ack)

	def SendAcks(self):
		"""
		Causes all subsequent I2C read operations to respond with an acknowledgement.
		"""
		return _mpsse.mpsse_send_acks(self.context)

	def SendNacks(self):
		"""
		Causes all subsequent I2C read operations to respond with a no-acknowledgement.
		"""
		return _mpsse.mpsse_send_nacks(self.context)

	def GetAck(self):
		"""
		Returns the last received ACK bit.
		Returns 0 for ACK, 1 for NACK.
		"""
		return _mpsse.mpsse_get-ack(self.context)

	def PinHigh(self, pin):
		"""
		Sets the specified GPIO pin high.
		The pin can be GPIO pin 0 - 11.
		"""
		if _mpsse.mpsse_pin_high(self.context, pin) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def PinLow(self, pin):
		"""
		Sets the specified GPIO pin low.
		The Pin can be GPIO pin 0 - 11.
		"""
		if _mpsse.mpsse_pin_low(self.context, pin) == MPSSE_FAIL:
			raise Exception, self.ErrorString()
		return MPSSE_OK

	def ReadPins(self):
		"""
		Reads the current state of the chip's pins. For use in BITBANG mode only.
		Returns a byte with the corresponding pin's bits set.
		"""
		return _mpsse.mpsse_read_pins(self.context)

	def PinState(self, pin, state=-1):
		"""
		Checks the current state of the pins. For use in BITBANG mode only.
		Set pin to the pin number you want to check. 
		State is the value returned by ReadPins. If not specified, ReadPins will be called automatically.
		"""
		return _mpsse.mpsse_pin_state(self.context, pin, state)

	def ClockUntilHigh(self):
		"""
		Toggles the clock without data transfer until GPIO1 is pulled high.
		"""
		return _mpsse.mpsse_clock_until_high(self.context)

	def ClockUntilLow(self):
		"""
		Toggles the clock without data transfer until GPIO1 is pulled low.
		"""
		return _mpsse.mpsse_clock_until_low(self.context)

	def ToggleClock(self, count):
		"""
		Toggle the clock line count times without transferring any data.
		"""
		return _mpsse.mpsse_toggle_clock(self.context, count, gpio)

	def ToggleClockX8(self, count, gpio=-1):
		"""
		Toggle the clock line 8*count times without transferring any data.
		
		If gpio is 0, then the clock output will be interrupted by pulling GPIOL1 low.
		If gpio is 1, then the clock output will be interrupted by pulling GPIOL1 high.
		If gpio is -1, GPIOL1 is ignored (default).
		"""
		return _mpsse.mpsse_toggle_clock_x8(self.context, count, gpio)

	def Tristate(self):
		"""
		Puts all I/O pins into a tristate mode (FT232H only).
		"""
		return _mpsse.mpsse_tristate(self.context)

	def MCUWrite(self, data, address=0):
		"""
		Writes data starting at address using MCU emulation mode.
		"""
		return _mpsse.mpsse_mcu_write(self.context, data, address)

	def MCURead(self, size, address=0):
		"""
		Reads size bytes of data starting at address using MCU emulation mode.
		"""
		return _mpsse.mpsse_mcu_read(self.context, size, address)

	def Version(self):
		"""
		Returns the libmpsse version number.
		High nibble is major, low nibble is minor.
		"""
		return _mpsse.mpsse_version()
