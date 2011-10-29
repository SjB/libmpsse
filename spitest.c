#include <stdio.h>
#include <stdlib.h>
#include "mpsse.h"

#define FREQUENCY	30000000		// 30MHz
#define CHIP_SIZE	0x100000		// 1MB
#define SAVE_FILE	"data.dump"		// Output file

int main(void)
{
	FILE *fp = NULL;
	char *chipid = NULL, *rxdata = NULL;

	if(MPSSE(SPI3, FREQUENCY, MSB) == MPSSE_OK)
	{
		/* Lowering the timeout increases throughput, but setting it too low will result in USB transfer errors */
		SetTimeouts(30);

		printf("Connected to %s\n", mpsse.description);
		printf("SPI frequency: %dHz\n\n", mpsse.clock);

		/* Read the chip's 3 byte JEDEC ID */
		Start();
		Write("\x9F", 1);
		chipid = Read(3);
		Stop();

		printf("Chip Manufacturer Code: 0x%.2X\n", (chipid[0] & 0xFF));
		printf("Chip Memory Type:       0x%.2X\n", (chipid[1] & 0xFF));
		printf("Chip Capcity Code:      0x%.2X\n\n", (chipid[2] & 0xFF));

		printf("Reading data...\n");
		
		/* Start the read by sending the read command (0x03) followed by a three byte start address (0x000000) */
		Start();		
		Write("\x03\x00\x00\x00", 4);
		rxdata = Read(CHIP_SIZE);
		Stop();

		/* Save data to file */
		fp = fopen(SAVE_FILE, "wb");
		fwrite(rxdata, 1, CHIP_SIZE, fp);
		fclose(fp);

		printf("\nSaved %d bytes to %s\n", CHIP_SIZE, SAVE_FILE);

		/* The caller is responsible for freeing the pointers returned by Read() */
		free(chipid);
		free(rxdata);
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	/* Always close out the chip properly */
	Close();

	return EXIT_SUCCESS;
}
