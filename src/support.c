/*
 * Internal functions used by libmpsse. 
 */

#include <string.h>
#include <ftdi.h>
#include "mpsse.h"
#include "support.h"

/* Write data to the FTDI chip */
int raw_write(unsigned char *buf, int size)
{
        int retval = MPSSE_FAIL;

        if(mpsse.mode && ftdi_write_data(&mpsse.ftdi, buf, size) == size)
        {
                retval = MPSSE_OK;
        }

	return retval;
}

/* Read data from the FTDI chip */
int raw_read(unsigned char *buf, int size)
{
	int n = 0, r = 0;

	if(mpsse.mode)
	{
		while(n < size)
		{
			r = ftdi_read_data(&mpsse.ftdi, buf, size);
			if(r < 0) break;
			n += r;
		}

		/* Make sure the buffers are cleared after a read or subsequent reads may fail */
		ftdi_usb_purge_rx_buffer(&mpsse.ftdi);
	}

	return n;
}

/* Convert a frequency to a clock divisor */
uint16_t freq2div(uint32_t system_clock, uint32_t freq)
{
	return (((system_clock / freq) / 2) - 1);
}

/* Convert a clock divisor to a frequency */
uint32_t div2freq(uint32_t system_clock, uint16_t div)
{
	return (system_clock / ((1 + div) * 2));
}

/* Builds a buffer of commands + data blocks */
unsigned char *build_block_buffer(uint8_t cmd, unsigned char *data, int size, int *buf_size)
{
	unsigned char *buf = NULL;
       	int i = 0, j = 0, k = 0, dsize = 0, num_blocks = 0, total_size = 0, xfer_size = TRANSFER_SIZE;
 	uint16_t rsize = 0;

	*buf_size = 0;

	/* Data block size is 1 in I2C */
	if(mpsse.mode == I2C)
	{
		xfer_size = 1;
	}

	num_blocks = (size / xfer_size);
	if(size % xfer_size)
	{
		num_blocks++;
	}

	/* The total size of the data will be the data size + the write command */
        total_size = size + (CMD_SIZE * num_blocks);

	/* In I2C we have to add 3 additional commands per data block */
	if(mpsse.mode == I2C)
	{
		total_size += (CMD_SIZE * 3 * num_blocks);
	}

        buf = malloc(total_size);
        if(buf)
        {
                memset(buf, 0, total_size);

		for(j=0; j<num_blocks; j++)
		{
			dsize = size - k;
			if(dsize > xfer_size)
			{
				dsize = xfer_size;
			}

			/* The reported size of this block is block size - 1 */
			rsize = dsize - 1;

			/* For I2C we need to ensure that the clock pin is set low prior to clocking out data */
			if(mpsse.mode == I2C)
			{
				buf[i++] = SET_BITS_LOW;
				buf[i++] = mpsse.pstart & ~SK;
				
				/* On receive, we need to ensure that the data out line is set as an input to avoid contention on the bus */
				if(cmd == mpsse.rx)
				{
					buf[i++] = mpsse.tris & ~DO;
				}
				else
				{
					buf[i++] = mpsse.tris;
				}
			}

			/* Copy in the command for this block */
			buf[i++] = cmd;
			buf[i++] = (rsize & 0xFF);
			buf[i++] = ((rsize >> 8) & 0xFF);

			/* On a write, copy the data to transmit after the command */
			if(cmd == mpsse.tx)
			{
				memcpy(buf+i, data+k, dsize);

				/* i == offset into buf */
				i += dsize;
				/* k == offset into data */
				k += dsize;
			}

			/* In I2C mode we need to clock one ACK bit after each byte */
			if(mpsse.mode == I2C)
			{
				/* If we are receiving data, then we need to clock out an ACK for each byte */
				if(cmd == mpsse.rx)
				{
					buf[i++] = SET_BITS_LOW;
					buf[i++] = mpsse.pstart & ~SK;
					buf[i++] = mpsse.tris;

					buf[i++] = mpsse.tx | MPSSE_BITMODE;
					buf[i++] = 0;
					buf[i++] = mpsse.tack;
				}
				/* If we are sending data, then we need to clock in an ACK for each byte */
				else if(cmd == mpsse.tx)
				{
					/* Need to make data out an input to avoid contention on the bus when the slave sends an ACK */
					buf[i++] = SET_BITS_LOW;
					buf[i++] = mpsse.pstart & ~SK;
					buf[i++] = mpsse.tris & ~DO;

					buf[i++] = mpsse.rx | MPSSE_BITMODE;
					buf[i++] = 0;
					buf[i++] = SEND_IMMEDIATE;
				}
			}
		}

		*buf_size = i;
	}

	return buf;
}
