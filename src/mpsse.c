/*
 * LibMPSSE source.
 */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ftdi.h>
#include "mpsse.h"
#include "support.h"

/* List of known FT2232-based devices */
struct vid_pid supported_devices[] = { 
			{ 0x0403, 0x6010, "FT2232 Future Technology Devices International, Ltd" }, 
			{ 0x0403, 0x6011, "FT4232 Future Technology Devices International, Ltd" }, 
			{ 0x0403, 0x6014, "FT232H Future Technology Devices International, Ltd" },

			/* These devices are based on FT2232 chips, but have not been tested. */
			{ 0x0403, 0x8878, "Bus Blaster v2 (channel A)" },
			{ 0x0403, 0x8879, "Bus Blaster v2 (channel B)" },
			{ 0x0403, 0xBDC8, "Turtelizer JTAG/RS232 Adapter A" },
			{ 0x0403, 0xCFF8, "Amontec JTAGkey" },
			{ 0x15BA, 0x0003, "Olimex Ltd. OpenOCD JTAG" },
			{ 0x15BA, 0x0004, "Olimex Ltd. OpenOCD JTAG TINY" },
			
			{ 0, 0, NULL }
};

int last_index = 0;

/*
 * Opens and initializes an FTDI device for the specified mode.
 * 
 * @mode      - Mode to open the device in. One of enum modes.
 * @freq      - Clock frequency to use for the specified mode.
 * @endianess - Specifies how data is clocked in/out (MSB, LSB).
 *
 * Returns MPSSE descriptor on success.
 * Returns MPSSE_FAIL on failure.
 */
int MPSSE(enum modes mode, int freq, int endianess)
{
	int i = 0, index = MPSSE_FAIL;
	static int start_index;

	for(i=start_index; ((supported_devices[i].vid != 0) && (index == MPSSE_FAIL)); i++)
	{
		if((index = Open(supported_devices[i].vid, supported_devices[i].pid, IFACE_A, mode, freq, endianess)) != MPSSE_FAIL)
		{
			mpsse[index].description = supported_devices[i].description;
			start_index = index + 1;
		}
	}
	
	return index;
}

/* 
 * Open device by VID/PID 
 *
 * @vid       - Device vendor ID.
 * @pid       - Device product ID.
 * @interface - FTDI interface to use (IFACE_A - IFACE_D)
 * @mode      - MPSSE mode, one of enum modes.
 * @freq      - Clock frequency to use for the specified mode.
 * @endianess - SPecifies how data is clocked in/out (MSB, LSB).
 *
 * Returns MPSSE descriptor on success.
 * Returns MPSSE_FAIL on failure.
 */
int Open(int vid, int pid, int interface, enum modes mode, int freq, int endianess)
{
	int status = 0, this_index = MPSSE_FAIL, retval = MPSSE_FAIL;
	static int gindex;

	memset((void *) &mpsse, 0, sizeof(struct globule));

	/* ftdilib initialization */
	if(ftdi_init(&mpsse[gindex].ftdi) == 0)
	{
		this_index = gindex;
		last_index = gindex;
		gindex++;

		/* Open the specified device */
		if(ftdi_usb_open(&mpsse[gindex].ftdi, vid, pid) == 0)
		{
			mpsse[gindex].mode = mode;
			mpsse[gindex].vid = vid;
			mpsse[gindex].pid = pid;
			mpsse[gindex].status = STOPPED;

			/* Set the appropriate transfer size for the requested protocol */
			if(mpsse[gindex].mode == I2C)
			{
				mpsse[gindex].xsize = I2C_TRANSFER_SIZE;
			}
			else
			{
				mpsse[gindex].xsize = SPI_TRANSFER_SIZE;
			}

			status |= ftdi_set_interface(&mpsse[gindex].ftdi, interface);
			status |= ftdi_usb_reset(&mpsse[gindex].ftdi);
			status |= ftdi_set_latency_timer(&mpsse[gindex].ftdi, LATENCY_MS);
			status |= ftdi_write_data_set_chunksize(&mpsse[gindex].ftdi, CHUNK_SIZE);
			status |= ftdi_read_data_set_chunksize(&mpsse[gindex].ftdi, CHUNK_SIZE);
			status |= ftdi_set_bitmode(&mpsse[gindex].ftdi, 0, BITMODE_RESET);
			status |= ftdi_set_bitmode(&mpsse[gindex].ftdi, 0, BITMODE_MPSSE);
			status |= ftdi_usb_purge_buffers(&mpsse[gindex].ftdi);

			if(status == 0)
			{
				if(SetClock(gindex, freq) == MPSSE_OK)
				{
					/* Set the read and write timeout periods */
					set_timeouts(gindex, USB_TIMEOUT);

					if(SetMode(gindex, mode, endianess) == MPSSE_OK)
					{
						retval = this_index;
					}
				}
			}

		}
	}

	return retval;
}

/* 
 * Closes the device and deinitializes libftdi.
 *
 * @index - MPSSE descriptor.
 *
 * Returns void.
 */
void Close(int index)
{
	if(index != MPSSE_FAIL && mpsse[index].mode)
	{
		ftdi_set_bitmode(&mpsse[index].ftdi, 0, BITMODE_RESET);
		ftdi_usb_close(&mpsse[index].ftdi);
		ftdi_deinit(&mpsse[index].ftdi);
	}

	return;
}

/*
 * Sets the appropriate transmit and receive commands based on the requested mode and byte order.
 *
 * @mode      - The desired mode, as listed in enum modes.
 * @endianess - MPSSE_MSB or MPSSE_LSB.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int SetMode(int index, enum modes mode, int endianess)
{
	int retval = MPSSE_OK, i = 0, setup_commands_size = 0;
	char buf[CMD_SIZE] = { 0 };
	char setup_commands[CMD_SIZE*4] = { 0 };

	/* Read and write commands need to include endianess */
	mpsse[index].tx = MPSSE_DO_WRITE | endianess;
	mpsse[index].rx = MPSSE_DO_READ | endianess;

	/* Clock, data out, chip select pins are outputs; all others are inputs. */
	mpsse[index].tris = DEFAULT_TRIS;

	/* Clock and chip select pins idle high; all others are low */
	mpsse[index].pidle = mpsse[index].pstart = mpsse[index].pstop = DEFAULT_PORT;

	/* During reads and writes the chip select pin is brought low */
	mpsse[index].pstart &= ~CS;

	/* Disable FTDI internal loopback */
        SetLoopback(index, 0);

	SetAck(index, 0);

	/* Ensure adaptive clock is disabled */
	setup_commands[setup_commands_size++] = DISABLE_ADAPTIVE_CLOCK;

	switch(mode)
	{
		case SPI0:
			/* SPI mode 0 clock idles low */
			mpsse[index].pidle &= ~SK;
			mpsse[index].pstart &= ~SK;
			mpsse[index].pstop &= ~SK;
			/* SPI mode 0 propogates data on the falling edge and read data on the rising edge of the clock */
			mpsse[index].tx |= MPSSE_WRITE_NEG;
			mpsse[index].rx &= ~MPSSE_READ_NEG;
			break;
		case SPI3:
			/* SPI mode 3 clock idles high */
			mpsse[index].pidle |= SK;
			mpsse[index].pstart |= SK;
			/* Keep the clock low while the CS pin is brought high to ensure we don't accidentally clock out an extra bit */
			mpsse[index].pstop &= ~SK;
			/* SPI mode 3 propogates data on the falling edge and read data on the rising edge of the clock */
			mpsse[index].tx |= MPSSE_WRITE_NEG;
			mpsse[index].rx &= ~MPSSE_READ_NEG;
			break;
		case SPI1:
			/* SPI mode 1 clock idles low */
			mpsse[index].pidle &= ~SK;
			/* Since this mode idles low, the start condition should ensure that the clock is low */
			mpsse[index].pstart &= ~SK;
			/* Even though we idle low in this mode, we need to keep the clock line high when we set the CS pin high to prevent
			 * an unintended clock cycle from being sent by the FT2232. This way, the clock goes high, but does not go low until
			 * after the CS pin goes high.
			 */
			mpsse[index].pstop |= SK;
			/* Data read on falling clock edge */
			mpsse[index].rx |= MPSSE_READ_NEG;
			mpsse[index].tx &= ~MPSSE_WRITE_NEG;
			break;
		case SPI2:
			/* SPI 2 clock idles high */
			mpsse[index].pidle |= SK;
			mpsse[index].pstart |= SK;
			mpsse[index].pstop |= SK;
			/* Data read on falling clock edge */
			mpsse[index].tx &= ~MPSSE_WRITE_NEG;
			mpsse[index].rx |= MPSSE_READ_NEG;
			break;
		case I2C:
			/* I2C propogates data on the falling clock edge and reads data on the falling (or rising) clock edge */
			mpsse[index].tx |= MPSSE_WRITE_NEG;
			mpsse[index].rx &= ~MPSSE_READ_NEG;
			/* In I2C, both the clock and the data lines idle high */
			mpsse[index].pidle |= DO | DI;
			/* I2C start bit == data line goes from high to low while clock line is high */
			mpsse[index].pstart &= ~DO & ~DI;
			/* I2C stop bit == data line goes from low to high while clock line is high - set data line low here, so the transition to the idle state triggers the stop condition. */
			mpsse[index].pstop &= ~DO & ~DI;
			/* Enable three phase clock to ensure that I2C data is available on both the rising and falling clock edges */
			setup_commands[setup_commands_size++] = ENABLE_3_PHASE_CLOCK;
			break;
		case GPIO:
			break;
		default:
			retval = MPSSE_FAIL;
	}

	/* Send any setup commands to the chip */
	if(retval == MPSSE_OK && setup_commands_size > 0)
	{
		retval = raw_write(index, (unsigned char *) &setup_commands, setup_commands_size);
	}

	if(retval == MPSSE_OK)
	{
		/* Set the idle pin states */
		set_bits_low(index, mpsse[index].pidle);

		/* All GPIO pins are inputs, pulled low */
		mpsse[index].trish = 0xFF;
		mpsse[index].gpioh = 0x00;

                buf[i++] = SET_BITS_HIGH;
                buf[i++] = mpsse[index].gpioh;
                buf[i++] = mpsse[index].trish;

		retval = raw_write(index, (unsigned char *) &buf, i);
	}

	return retval;
}

/* 
 * Sets the appropriate divisor for the desired clock frequency.
 *
 * @freq - Desired clock frequency in hertz.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int SetClock(int index, uint32_t freq)
{
	int retval = MPSSE_FAIL;
	uint32_t system_clock = 0;
	uint16_t divisor = 0;
	char buf[CMD_SIZE] = { 0 };

	if(freq > SIX_MHZ)
	{
		buf[0] = TCK_X5;
		system_clock = SIXTY_MHZ;
	}
	else
	{
		buf[0] = TCK_D5;
		system_clock = TWELVE_MHZ;
	}
	
	if(raw_write(index, (unsigned char *) &buf, 1) == MPSSE_OK)
	{
		if(freq <= 0)
		{
			divisor = 0xFFFF;
		}
		else
		{
			divisor = freq2div(system_clock, freq);
		}

		buf[0] = TCK_DIVISOR;
		buf[1] = (divisor & 0xFF);
		buf[2] = ((divisor >> 8) & 0xFF);

		if(raw_write(index, (unsigned char *) &buf, 3) == MPSSE_OK)
		{
			mpsse[index].clock = div2freq(system_clock, divisor);
			retval = MPSSE_OK;
		}
	}

	return retval;
}

/* 
 * Retrieves the last error string from libftdi
 *
 * Returns a pointer to the last error string.
 */
char *ErrorString(int index)
{
	if(index == MPSSE_FAIL)
	{
		index = last_index;
	}

        return ftdi_get_error_string(&mpsse[index].ftdi);
}

/* 
 * Gets the currently configured clock rate.
 *
 * Returns the existing clock rate in hertz.
 */
int GetClock(int index)
{
	return mpsse[index].clock;
}

/*
 * Returns the vendor ID of the FTDI chip.
 */
int GetVid(int index)
{
	return mpsse[index].vid;
}

/*
 * Returns the product ID of the FTDI chip.
 */
int GetPid(int index)
{
	return mpsse[index].pid;
}

/*
 * Returns the description of the FTDI chip, if any.
 */
char *GetDescription(int index)
{
	return mpsse[index].description;
}

/* 
 * Enable / disable internal loopback.
 * 
 * @enable - Zero to disable loopback, 1 to enable loopback.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int SetLoopback(int index, int enable)
{
	char buf[1] = { 0 };

	if(enable)
	{
		buf[0] = LOOPBACK_START;
	}
	else
	{
		buf[0] = LOOPBACK_END;
	}

	return raw_write(index, (unsigned char *) &buf, 1);
}

/*
 * Sets the idle state of the chip select pin. CS idles high by default.
 *
 * @idle - Set to 1 to idle high, 0 to idle low.
 *
 * Returns void.
 */
void SetCSIdle(int index, int idle)
{
	if(idle > 0)
	{
		/* Chip select idles high, active low */
		mpsse[index].pidle |= CS;
		mpsse[index].pstop |= CS;
		mpsse[index].pstart &= ~CS;
	}
	else
	{
		/* Chip select idles low, active high */
		mpsse[index].pidle &= ~CS;
		mpsse[index].pstop &= ~CS;
		mpsse[index].pstart |= CS;
	}

	return;
}

/*
 * Send data start condition.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Start(int index)
{
	int status = MPSSE_OK;

	mpsse[index].status = STARTED;

	if(mpsse[index].mode == I2C)
	{
		/* Set the default pin states while the clock is low in case this is an I2C repeated start condition */
		status |= set_bits_low(index, (mpsse[index].pidle & ~SK));

		/* Make sure the pins are in their default idle state */
		status |= set_bits_low(index, mpsse[index].pidle);
	}

	/* Set the start condition */
	status |= set_bits_low(index, mpsse[index].pstart);

	/* 
	 * Hackish work around to properly support SPI mode 3.
	 * SPI3 clock idles high, but needs to be set low before sending out 
	 * data to prevent unintenteded clock glitches from the FT2232.
	 */
	if(mpsse[index].mode == SPI3)
        {
		status |= set_bits_low(index, (mpsse[index].pstart & ~SK));
        }
	/*
	 * Hackish work around to properly support SPI mode 1.
	 * SPI1 clock idles low, but needs to be set high before sending out
	 * data to preven unintended clock glitches from the FT2232.
	 */
	else if(mpsse[index].mode == SPI1)
	{
		status |= set_bits_low(index, (mpsse[index].pstart | SK));
	}

	return status;
}

/*
 * Send data out via the selected serial protocol.
 *
 * @data - Buffer of data to send.
 * @size - Size of data.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Write(int index, char *data, int size)
{
	unsigned char *buf = NULL;
	int retval = MPSSE_FAIL, buf_size = 0, txsize = 0, n = 0;

	if(mpsse[index].mode)
	{
		while(n < size)
		{
			txsize = size - n;
			if(txsize > mpsse[index].xsize)
			{
				txsize = mpsse[index].xsize;
			}

			/* 
			 * For I2C we need to send each byte individually so that we can 
			 * read back each individual ACK bit, so set the transmit size to 1.
			 */
			if(mpsse[index].mode == I2C)
			{
				txsize = 1;
			}

			buf = build_block_buffer(index, mpsse[index].tx, (unsigned char *) data+n, txsize, &buf_size);
			if(buf)
			{	
				retval = raw_write(index, buf, buf_size);
				n += txsize;
				free(buf);

				if(retval == MPSSE_FAIL)
				{
					break;
				}
			
				/* Read in the ACK bit and store it in mpsse[index].rack */
				if(mpsse[index].mode == I2C)
				{
					raw_read(index, (unsigned char *) &mpsse[index].rack, 1);
				}
			}
			else
			{
				break;
			}
		}
	}

	if(retval == MPSSE_OK && n == size)
	{
		retval = MPSSE_OK;
	}

	return retval;
}

/*
 * Reads data over the selected serial protocol.
 * 
 * @size  - Number of bytes to read.
 *
 * Returns a pointer to the read data on success.
 * Returns NULL on failure.
 */
#ifdef SWIGPYTHON
swig_string_data Read(int index, int size)
#else
char *Read(int index, int size)
#endif
{
	unsigned char *data = NULL, *buf = NULL;
	char sbuf[SPI_TRANSFER_SIZE] = { 0 };
	int n = 0, rxsize = 0, data_size = 0;

	if(mpsse[index].mode)
	{
		buf = malloc(size);
		if(buf)
		{
			memset(buf, 0, size);

			while(n < size)
			{
				rxsize = size - n;
				if(rxsize > mpsse[index].xsize)
				{
					rxsize = mpsse[index].xsize;
				}

				data = build_block_buffer(index, mpsse[index].rx, (unsigned char *) &sbuf, rxsize, &data_size);
				if(data)
				{
					if(raw_write(index, data, data_size) == MPSSE_OK)
					{
						n += raw_read(index, buf+n, rxsize);
					}
					else
					{
						break;
					}
					
					free(data);
				}
				else
				{
					break;
				}
			}
		}
	}

#ifdef SWIGPYTHON
	swig_string_data sdata = { 0 };
	sdata.size = n;
	sdata.data = (char *) buf;
	return sdata;
#else
	return (char *) buf;
#endif
}

/*
 * Returns the last received ACK bit.
 */
int GetAck(int index)
{
	return (mpsse[index].rack & 0x01);
}

/*
 * Sets the transmitted ACK bit.
 *
 * @index - MPSSE descriptor.
 * @ack   - 1 to send ACKs, 0 to send NACKs.
 *
 * Returns void.
 */
void SetAck(int index, int ack)
{
	if(ack)
	{
		mpsse[index].tack = 0xFF;
	}
	else
	{
		mpsse[index].tack = 0x00;
	}

	return;
}

/*
 * Send data stop condition.
 *
 * @index - MPSSE descriptor.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Stop(int index)
{
	int retval = MPSSE_OK;

	mpsse[index].status = STOPPED;

	/* In I2C mode, we need to ensure that the data line goes low while the clock line is low to avoid sending an inadvertent start condition */
	if(mpsse[index].mode == I2C)
	{
		retval |= set_bits_low(index, (mpsse[index].pidle & ~DO & ~SK));
	}

	/* Send the stop condition */
	retval |= set_bits_low(index, mpsse[index].pstop);

	if(retval == MPSSE_OK)
	{
		/* Restore the pins to their idle states */
		retval |= set_bits_low(index, mpsse[index].pidle);
	}

	return retval;
}

/* Sets the specified pin high */
int PinHigh(int index, int pin)
{
	return gpio_write(index, pin, HIGH);
}

/* Sets the specified pin low */
int PinLow(int index, int pin)
{
	return gpio_write(index, pin, LOW);
}
