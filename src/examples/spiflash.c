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
	
	if(MPSSE(SPI0, THIRTY_MHZ, MSB) == MPSSE_OK)
	{
		printf("%s initialized at %dHz (SPI mode 0)\n", GetDescription(), GetClock());
		
		Start();
		Write(RCMD, sizeof(RCMD) - 1);
		data = Read(SIZE);
		Stop();
		
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
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	Close();

	return retval;
}
