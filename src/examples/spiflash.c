#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define SIZE	0x100000		// Size of SPI flash device: 1MB
#define RCMD	"\x03\x00\x00\x00"	// Standard SPI flash read command (0x03) followed by starting address (0x000000)
#define FOUT	"flash.bin"		// Output file

int main(void)
{
	FILE *fp = NULL;
	char *data = NULL;
	int retval = EXIT_FAILURE;
	struct mpsse_context *flash = NULL;
	
	if((flash = MPSSE.SPI.Open(SPI0, TWELVE_MHZ, MSB)) != NULL && flash->open)
	{
		printf("%s initialized at %dHz (SPI mode 0)\n", MPSSE.GetDescription(flash), MPSSE.GetClock(flash));
		
		MPSSE.SPI.Start(flash);
		MPSSE.SPI.Write(flash, RCMD, sizeof(RCMD) - 1);
		data = MPSSE.SPI.Read(flash, SIZE);
		MPSSE.SPI.Stop(flash);
		
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
	else
	{
		printf("Failed to initialize MPSSE: %s\n", MPSSE.ErrorString(flash));
	}

	MPSSE.SPI.Close(flash);

	return retval;
}
