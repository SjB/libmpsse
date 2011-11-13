/*
 * LibMPSSE source.
 */

#include <string.h>
#include <stdint.h>
#include <ftdi.h>
#include "mpsse.h"
#include "support.h"

/* List of known FT2232-based devices */
struct vid_pid supported_devices[] = { 
			{ 0x0403, 0x6010, "FT2232 Future Technology Devices International, Ltd" }, 
			{ 0x0403, 0x6011, "FT4232 Future Technology Devices International, Ltd" }, 
			{ 0x0403, 0x6014, "FT232H Future Technology Devices International, Ltd" },
			{ 0x0403, 0x8878, "Bus Blaster v2 (channel A)" },
			{ 0x0403, 0x8879, "Bus Blaster v2 (channel B)" },
			{ 0x0403, 0xBDC8, "Turtelizer JTAG/RS232 Adapter A" },
			{ 0x0403, 0xCFF8, "Amontec JTAGkey" },
			{ 0x15BA, 0x0003, "Olimex Ltd. OpenOCD JTAG" },
			{ 0x15BA, 0x0004, "Olimex Ltd. OpenOCD JTAG TINY" },
			{ 0, 0, NULL }
};


/*
 * Opens and initializes an FTDI device for the specified mode.
 * 
 * @mode      - Mode to open the device in. One of enum modes.
 * @freq      - Clock frequency to use for the specified mode.
 * @endianess - Specifies how data is clocked in/out (MSB, LSB).
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int MPSSE(enum modes mode, int freq, int endianess)
{
	int i = 0, retval = MPSSE_FAIL;

	for(i=0; ((supported_devices[i].vid != 0) && (retval == MPSSE_FAIL)); i++)
	{
		if((retval = Open(supported_devices[i].vid, supported_devices[i].pid, mode, freq, endianess)) == MPSSE_OK)
		{
			mpsse.description = supported_devices[i].description;
		}
	}
	
	return retval;
}

/* 
 * Open device by VID/PID 
 *
 * @vid       - Device vendor ID.
 * @pid       - Device product ID.
 * @mode      - MPSSE mode, one of enum modes.
 * @freq      - Clock frequency to use for the specified mode.
 * @endianess - SPecifies how data is clocked in/out (MSB, LSB).
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Open(int vid, int pid, enum modes mode, int freq, int endianess)
{
	int status = 0, timeout = 0, retval = MPSSE_FAIL;

	memset((void *) &mpsse, 0, sizeof(mpsse));

	/* ftdilib initialization */
	if(ftdi_init(&mpsse.ftdi) == 0)
	{

		/* Open the specified device */
		if(ftdi_usb_open(&mpsse.ftdi, vid, pid) == 0)
		{
			mpsse.mode = mode;

			status |= ftdi_set_interface(&mpsse.ftdi, INTERFACE_A);
			status |= ftdi_usb_reset(&mpsse.ftdi);
			status |= ftdi_set_latency_timer(&mpsse.ftdi, LATENCY_MS);
			status |= ftdi_write_data_set_chunksize(&mpsse.ftdi, TRANSFER_SIZE);
			status |= ftdi_read_data_set_chunksize(&mpsse.ftdi, TRANSFER_SIZE);
			status |= ftdi_set_bitmode(&mpsse.ftdi, 0, BITMODE_RESET);
			status |= ftdi_set_bitmode(&mpsse.ftdi, 0, BITMODE_MPSSE);
			status |= ftdi_usb_purge_buffers(&mpsse.ftdi);

			if(status == 0)
			{
				if(SetClock(freq) == MPSSE_OK)
				{
					timeout = (int) ((float) TIMEOUT_MS / ((float) freq / (float) TIMEOUT_DIVISOR));
					if(timeout > MAX_TIMEOUT_MS)
					{
						timeout = MAX_TIMEOUT_MS;
					}

					/* Set the read and write timeout periods */
					SetTimeouts(timeout);

					retval = SetMode(mode, endianess);
				}
			}

		}
	}

	return retval;
}

/* 
 * Closes the device and deinitializes libftdi.
 *
 * Returns void.
 */
void Close(void)
{
	if(mpsse.mode)
	{
		ftdi_usb_close(&mpsse.ftdi);
		ftdi_deinit(&mpsse.ftdi);
	}

	return;
}

/* 
 * Retrieves the last error string from libftdi
 *
 * Returns a pointer to the last error string.
 */
char *ErrorString(void)
{
	return ftdi_get_error_string(&mpsse.ftdi);
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
int SetMode(enum modes mode, int endianess)
{
	int retval = MPSSE_OK, i = 0, setup_commands_size = 0;
	char buf[CMD_SIZE*2] = { 0 };
	char setup_commands[CMD_SIZE*4] = { 0 };

	/* Read and write commands need to include endianess */
	mpsse.tx = MPSSE_DO_WRITE | endianess;
	mpsse.rx = MPSSE_DO_READ | endianess;

	/* Clock, data out, chip select pins are outputs; all others are inputs. */
	mpsse.tris = DEFAULT_TRIS;

	/* Clock and chip select pins idle high; all others are low */
	mpsse.pidle = mpsse.pstart = mpsse.pstop = DEFAULT_PORT;

	/* During reads and writes the chip select pin is brought low */
	mpsse.pstart &= ~CS;

	/* Disable FTDI internal loopback */
        SetLoopback(0);

	SetAck(0);

	/* Ensure adaptive clock is disabled */
	setup_commands[setup_commands_size++] = DISABLE_ADAPTIVE_CLOCK;

	switch(mode)
	{
		case SPI0:
			/* SPI mode 0 clock idles low */
			mpsse.pidle &= ~SK;
			/* Since this mode idles low, our "stop" condition should ensure that the clock is low */
			mpsse.pstop &= ~SK;
		case SPI3:
			/* Even though the clock for mode 3 idles high, we need to set it low as part of our start condition, else the FTDI chip will generate clock glitches every 8 bits */
			mpsse.pstart &= ~SK;
			/* SPI modes 0 and 3 propogate data on the falling edge and read data on the rising edge of the clock */
			mpsse.tx |= MPSSE_WRITE_NEG;
			mpsse.rx &= ~MPSSE_READ_NEG;
			break;
		case SPI1:
			/* SPI mode 1 clock idles low */
			mpsse.pidle &= ~SK;
			/* Since this mode idles low, our "stop" condition should ensure that the clock is low */
			mpsse.pstop &= ~SK;
		case SPI2:
			/* Even though the clock for mode 2 idles high, we need to set it low as part of our start condition, else the FTDI chip will generate clock glitches every 8 bits */
			mpsse.pstart &= ~SK;
			/* SPI modes 1 and 2 propogate data on the rising edge and read data on the falling edge of the clock */
			mpsse.tx &= ~MPSSE_WRITE_NEG;
			mpsse.rx |= MPSSE_READ_NEG;
			break;
		case I2C:
			/* I2C propogates data on the falling clock edge and reads data on the falling (or rising) clock edge */
			mpsse.tx |= MPSSE_WRITE_NEG;
			mpsse.rx &= ~MPSSE_READ_NEG;
			/* In I2C, both the clock and the data lines idle high */
			mpsse.pidle |= DO | DI;
			/* I2C start bit == data line goes from high to low while clock line is high */
			mpsse.pstart &= ~DO & ~DI;
			/* I2C stop bit == data line goes from low to high while clock line is high - set data line low here, so the transition to the idle state triggers the stop condition. */
			mpsse.pstop &= ~DO & ~DI;
			/* Enable three phase clock to ensure that I2C data is available on both the rising and falling clock edges */
			setup_commands[setup_commands_size++] = ENABLE_3_PHASE_CLOCK;
			break;
		default:
			retval = MPSSE_FAIL;
	}

	/* Send any setup commands to the chip */
	if(retval == MPSSE_OK && setup_commands_size > 0)
	{
		retval = raw_write((unsigned char *) &setup_commands, setup_commands_size);
	}

	if(retval == MPSSE_OK)
	{
		/* Set the idle pin states */
		buf[i++] = SET_BITS_LOW;
                buf[i++] = mpsse.pidle; 
                buf[i++] = mpsse.tris;

                buf[i++] = SET_BITS_HIGH;
                buf[i++] = 0x00;
                buf[i++] = 0x00;

		retval = raw_write((unsigned char *) &buf, i);
	}

	return retval;
}

/* 
 * Sets the read and write timeout periods for bulk usb data transfers.
 *
 * @timeout - Timeout period in milliseconds
 *
 * Returns void.
 */
void SetTimeouts(int timeout)
{
	if(mpsse.mode)
	{
        	mpsse.ftdi.usb_read_timeout = timeout;
        	mpsse.ftdi.usb_write_timeout = timeout;
	}
}

/* 
 * Sets the appropriate divisor for the desired clock frequency.
 *
 * @freq - Desired clock frequency in hertz.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int SetClock(uint32_t freq)
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
	
	if(raw_write((unsigned char *) &buf, 1) == MPSSE_OK)
	{
		divisor = freq2div(system_clock, freq);

		buf[0] = TCK_DIVISOR;
		buf[1] = (divisor & 0xFF);
		buf[2] = ((divisor >> 8) & 0xFF);

		if(raw_write((unsigned char *) &buf, 3) == MPSSE_OK)
		{
			mpsse.clock = div2freq(system_clock, divisor);
			retval = MPSSE_OK;
		}
	}

	return retval;
}

/* 
 * Gets the currently configured clock rate.
 *
 * Returns the existing clock rate in hertz.
 */
int GetClock(void)
{
	return mpsse.clock;
}

/*
 * Returns the vendor ID of the FTDI chip.
 */
int GetVid(void)
{
	return mpsse.vid;
}

/*
 * Returns the product ID of the FTDI chip.
 */
int GetPid(void)
{
	return mpsse.pid;
}

/*
 * Returns the description of the FTDI chip, if any.
 */
char *GetDescription(void)
{
	return mpsse.description;
}

/* 
 * Enable / disable internal loopback.
 * 
 * @enable - Zero to disable loopback, 1 to enable loopback.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int SetLoopback(int enable)
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

	return raw_write((unsigned char *) &buf, 1);
}

/*
 * Send data start condition.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Start(void)
{
	char buf[CMD_SIZE] = { 0 };
	int status = MPSSE_OK;

	if(mpsse.mode == I2C)
	{
		/* Set the default pin states while the clock is low in case this is an I2C repeated start condition */
		buf[0] = SET_BITS_LOW;
		buf[1] = mpsse.pidle & ~SK;
		buf[2] = mpsse.tris;
		status |= raw_write((unsigned char *) &buf, sizeof(buf));

		/* Make sure the pins are in their default idle state */
		buf[0] = SET_BITS_LOW;
		buf[1] = mpsse.pidle;
		buf[2] = mpsse.tris;
		status |= raw_write((unsigned char *) &buf, CMD_SIZE);
	}
	
	/* Set the start condition */
	buf[0] = SET_BITS_LOW;
	buf[1] = mpsse.pstart;
	buf[2] = mpsse.tris;
	status |= raw_write((unsigned char *) &buf, sizeof(buf));

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
int Write(char *data, int size)
{
	unsigned char *buf = NULL;
	int retval = MPSSE_FAIL, buf_size = 0, txsize = 0, n = 0;

	if(mpsse.mode)
	{
		while(n < size)
		{
			txsize = size - n;
			if(txsize > TRANSFER_SIZE)
			{
				txsize = TRANSFER_SIZE;
			}

			/* 
			 * For I2C we need to send each byte individually so that we can 
			 * read back each individual ACK bit, so set the transmit size to 1.
			 */
			if(mpsse.mode == I2C)
			{
				txsize = 1;
			}

			buf = build_block_buffer(mpsse.tx, (unsigned char *) data+n, txsize, &buf_size);
			if(buf)
			{	
				retval = raw_write(buf, buf_size);
				n += txsize;
				free(buf);

				if(retval == MPSSE_FAIL)
				{
					break;
				}
			
				/* Read in the ACK bit and store it in mpsse.rack */
				if(mpsse.mode == I2C)
				{
					raw_read((unsigned char *) &mpsse.rack, 1);
				}
			}
			else
			{
				break;
			}
		}
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
swig_string_data Read(int size)
#else
char *Read(int size)
#endif
{
	unsigned char *data = NULL, *buf = NULL;
	char sbuf[TRANSFER_SIZE] = { 0 };
	int n = 0, rxsize = 0, data_size = 0;

	if(mpsse.mode)
	{
		buf = malloc(size);
		if(buf)
		{
			memset(buf, 0, size);

			while(n < size)
			{
				rxsize = size - n;
				if(rxsize > TRANSFER_SIZE)
				{
					rxsize = TRANSFER_SIZE;
				}

				data = build_block_buffer(mpsse.rx, (unsigned char *) &sbuf, rxsize, &data_size);
				if(data)
				{
					if(raw_write(data, data_size) == MPSSE_OK)
					{
						n += raw_read(buf+n, rxsize);
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
	swig_string_data sdata= { 0 };
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
int GetAck(void)
{
	return (mpsse.rack & 0x01);
}

/*
 * Sets the transmitted ACK bit.
 *
 * Returns void.
 */
void SetAck(int ack)
{
	if(ack)
	{
		mpsse.tack = 0xFF;
	}
	else
	{
		mpsse.tack = 0x00;
	}

	return;
}

/*
 * Send data stop condition.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Stop(void)
{
	char buf[CMD_SIZE] = { 0 };
	int retval = MPSSE_OK;

	/* In I2C mode, we need to ensure that the data line goes low while the clock line is low to avoid sending an inadvertent start condition */
	if(mpsse.mode == I2C)
	{
		buf[0] = SET_BITS_LOW;
		buf[1] = mpsse.pidle & ~DO & ~SK;
		buf[2] = mpsse.tris;
		retval |= raw_write((unsigned char *) &buf, sizeof(buf));
	}

	/* Send the stop condition */
	buf[0] = SET_BITS_LOW;
	buf[1] = mpsse.pstop;
	buf[2] = mpsse.tris;
	retval |= raw_write((unsigned char *) &buf, sizeof(buf));

	if(retval == MPSSE_OK)
	{
		/* Restore the pins to their idle states */
		buf[0] = SET_BITS_LOW;
		buf[1] = mpsse.pidle;
		buf[2] = mpsse.tris;

		retval = raw_write((unsigned char *) &buf, sizeof(buf));
	}

	return retval;
}


