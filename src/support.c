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
        int retval = MPSSE_OK;

        if(ftdi_write_data(&mpsse.ftdi, buf, size) == size)
        {
                retval = MPSSE_OK;
        }
        
	return retval;
}

/* Read data from the FTDI chip */
int raw_read(unsigned char *buf, int size)
{
	int n = 0, r = 0;

	while(n < size)
	{
		r = ftdi_read_data(&mpsse.ftdi, buf, size);
		if(r < 0) break;
		n += r;
	}

	/* Make sure the buffers are cleared after a read or subsequent reads may fail */
	ftdi_usb_purge_rx_buffer(&mpsse.ftdi);

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
       	int i = 0, j = 0, k = 0, dsize = 0, num_blocks = 0, total_size = 0;
 	uint16_t rsize = 0;

	num_blocks = (size / TRANSFER_SIZE);
	if(size % TRANSFER_SIZE)
	{
		num_blocks++;
	}

	/* The total size of the data will be the data size + the write command */
        total_size = size + (CMD_SIZE * num_blocks);

        buf = malloc(total_size);
        if(buf)
        {
                memset(buf, 0, total_size);

		for(j=0; j<num_blocks; j++)
		{
			dsize = size - i;
			if(dsize > TRANSFER_SIZE)
			{
				dsize = TRANSFER_SIZE;
			}

			/* The reported size of this block is block size - 1 */
			rsize = dsize - 1;

			/* Copy in the command for this block */
			buf[i++] = cmd;
			buf[i++] = (rsize & 0xFF);
			buf[i++] = ((rsize >> 8) & 0xFF);
	
			/* Copy the data block after the command*/
			memcpy(buf+i, data+k, dsize);

			i += dsize;
			k += dsize;
		}

		*buf_size = total_size;
	}

	return buf;
}
