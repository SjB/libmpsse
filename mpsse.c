/*
 * LibMPSSE source.
 */

#include <string.h>
#include <stdint.h>
#include <ftdi.h>
#include "mpsse.h"
#include "support.h"

/* List of known supported device VIDs / PIDs */
struct vid_pid supported_devices[] = { 
			{ 0x0403, 0x6010, "FT2232H Future Technology Devices International, Ltd" }, 
			{ 0x0403, 0x6014, "FT232H Future Technology Devices International, Ltd" },
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
	int status = 0, retval = MPSSE_FAIL;

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
				mpsse.clock = SetClock(freq);
				if(mpsse.clock > 0)
				{
					/* Set the read and write timeout periods */
					SetTimeouts(TIMEOUT_MS / (freq/TIMEOUT_DIVISOR));
					
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
	char buf[6] = { 0 };
	char setup_commands[12] = { 0 };
	/* 
	 * Initialize default settings:
	 *
	 *	o Clock, data out and chip select pins are outputs; all others are inputs.
	 *	o Data is protogated on the rising clock edge and read on the falling clock edge.
	 *	o CS idles high and is brought low during reads and writes.
	 *	o Clock idles high.
	 * 	o FTDI internal loopback is disabled.
	 */ 
	configure_default_settings(endianess);
        SetLoopback(0);

	/* Ensure adaptive clock is disabled */
	setup_commands[setup_commands_size++] = DISABLE_ADAPTIVE_CLOCK;

	switch(mode)
	{
		case SPI0:
			/* SPI mode 0 clock idles low */
			mpsse.pidle &= ~SK;
		case SPI3:
			/* SPI modes 0 and 3 propogate data on the falling edge and read data on the rising edge of the clock */
			mpsse.tx |= MPSSE_WRITE_NEG;
			mpsse.rx &= ~MPSSE_READ_NEG;
			break;
		case SPI1:
			/* SPI mode 1 clock idles low */
			mpsse.pidle &= ~SK;
		case SPI2:
			/* SPI modes 1 and 2 propogate data on the rising edge and read data on the falling edge of the clock; nothing to do here. */
			break;
		case I2C:
			/* I2C propogates data on the falling clock edge and reads data on the rising clock edge */
			mpsse.tx |= MPSSE_WRITE_NEG;
			mpsse.rx &= ~MPSSE_READ_NEG;
			/* In I2C, both the clock and the data lines idle high */
			mpsse.pidle |= DO;
			/* I2C start bit == data line goes from high to low while clock line is high */
			mpsse.pstart &= ~DO;
			/* I2C stop bit == data line goes from low to high while clock line is high - set data line low here, so the transition to the idle state triggers the stop condition. */
			mpsse.pstop &= ~DO;
			/* FTDI documentation indicates that I2C should have the 3-phase clock enabled, but this seems to not work properly */
			//setup_commands[setup_commands_size++] = ENABLE_3_PHASE_CLOCK;
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
        mpsse.ftdi.usb_read_timeout = timeout;
        mpsse.ftdi.usb_write_timeout = timeout;
}

/* 
 * Sets the appropriate divisor for the desired clock frequency.
 *
 * @freq - Desired clock frequency in hertz.
 *
 * Returns the actual clock frequency in hertz on success.
 * Returns 0 on failure.
 */
uint32_t SetClock(uint32_t freq)
{
	uint32_t nfreq = 0, system_clock = 0;
	uint16_t divisor = 0;
	char buf[3] = { 0 };

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
			nfreq = div2freq(system_clock, divisor);
		}
	}

	return nfreq;
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
	char buf[3] = { 0 };

	/* Send the start condition */
	buf[0] = SET_BITS_LOW;
	buf[1] = mpsse.pstart;
	buf[2] = mpsse.tris;

	return raw_write((unsigned char *) &buf, sizeof(buf));
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

	while(n < size)
	{
		txsize = size - n;
		if(txsize > TRANSFER_SIZE)
		{
			txsize = TRANSFER_SIZE;
		}

		buf = build_block_buffer(mpsse.tx, (unsigned char *) data+n, txsize, &buf_size);
		if(buf)
		{	
			retval = raw_write(buf, buf_size);
			n += txsize;
			free(buf);
		}
		else
		{
			break;
		}
	}

	return retval;
}

/*
 * Reads data over the selected serial protocol.
 * 
 * @size  - Number of bytes to read, 1 - 65535.
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
				
				free(data);
			}
			else
			{
				break;
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
 * Send data stop condition.
 *
 * Returns MPSSE_OK on success.
 * Returns MPSSE_FAIL on failure.
 */
int Stop(void)
{
	char buf[3] = { 0 };
	int retval = MPSSE_FAIL;

	/* Send the stop condition */
	buf[0] = SET_BITS_LOW;
	buf[1] = mpsse.pstop;
	buf[2] = mpsse.tris;

	if(raw_write((unsigned char *) &buf, sizeof(buf)) == MPSSE_OK)
	{
		/* Restore the pins to their idle states */
		buf[0] = SET_BITS_LOW;
		buf[1] = mpsse.pidle;
		buf[2] = mpsse.tris;

		retval = raw_write((unsigned char *) &buf, sizeof(buf));
	}

	return retval;
}


