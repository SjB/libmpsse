/* 
 * Example code to read the contents of an I2C EEPROM chip.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define SIZE	0x8000		// Size of EEPROM chip (32KB)
#define WCMD	"\xA0\x00\x00"	// Write start address command
#define RCMD	"\xA1"		// Read command
#define FOUT	"eeprom.bin"	// Output file

int main(void)
{
	FILE *fp = NULL;
	char *data = NULL;
	int retval = EXIT_FAILURE;
	struct mpsse_context *eeprom = NULL;

	if((eeprom = MPSSE.I2C.Open(FOUR_HUNDRED_KHZ)) != NULL && eeprom->open)
	{
		printf("%s initialized at %dHz (I2C)\n", MPSSE.GetDescription(eeprom), MPSSE.GetClock(eeprom));
	
		/* Write the EEPROM start address */	
		MPSSE.I2C.Start(eeprom);
		MPSSE.I2C.Write(eeprom, WCMD, sizeof(WCMD) - 1);

		if(MPSSE.I2C.GetAck(eeprom) == ACK)
		{
			/* Send the EEPROM read command */
			MPSSE.I2C.Start(eeprom);
			MPSSE.I2C.Write(eeprom, RCMD, sizeof(RCMD) - 1);

			if(MPSSE.I2C.GetAck(eeprom) == ACK)
			{
				/* Read in SIZE bytes from the EEPROM chip */
				data = MPSSE.I2C.Read(eeprom, SIZE);
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
	
				/* Tell libmpsse to send NACKs after reading data */
				MPSSE.I2C.SendNacks(eeprom);

				/* Read in one dummy byte, with a NACK */
				MPSSE.I2C.Read(eeprom, 1);
			}
		}

		MPSSE.I2C.Stop(eeprom);
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", MPSSE.ErrorString(eeprom));
	}

	MPSSE.I2C.Close(eeprom);

	return retval;
}
