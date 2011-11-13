/* 
 * Example code to read the contents of an I2C EEPROM chip.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define SIZE	32		// Size of EEPROM chip
#define WRITE	"\xA0"		// EEPROM address + write bit
#define READ	"\xA1"		// EEPROM address + read bit
#define ADDR	"\x00\x00"	// Start address (0x0000)
#define FOUT	"eeprom.bin"	// Output file

int main(void)
{
	FILE *fp = NULL;
	char *data = NULL;
	int retval = EXIT_FAILURE;

	if(MPSSE(I2C, FOUR_HUNDRED_KHZ, MSB) == MPSSE_OK)
	{
		printf("%s initialized at %dHz (I2C)\n", GetDescription(), GetClock());
	
		/* Write the EEPROM start address */	
		Start();
		Write(WRITE, sizeof(WRITE) - 1);
		Write(ADDR,  sizeof(ADDR)  - 1);

		if(GetAck() == 0)
		{
			/* Send the EEPROM read command */
			Start();
			Write(READ, sizeof(READ) - 1);

			if(GetAck() == 0)
			{
				/* Read in SIZE bytes from the EEPROM chip */
				data = Read(SIZE);
				if(data)
				{
					fp = fopen(FOUT, "wb");
					if(fp)
					{
						fwrite(data, 1, SIZE, fp);
						fclose(fp);

						printf("Dumped %d bytes to %s\n", SIZE, FOUT);
						retval = EXIT_SUCCESS;
					}

					free(data);
				}
			}
		}

		Stop();
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	Close();

	return retval;
}
