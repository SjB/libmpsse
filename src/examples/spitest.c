#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define FREQUENCY	10000000	// 10MHz

int main(void)
{
	unsigned char txdata[1] = "\xAA";

	if(MPSSE(SPI3, FREQUENCY, MSB) == MPSSE_OK)
	{
		printf("SPI bus initialized at %dHz\n", GetClock());
		
		Start();

		if(Write((char *) &txdata, sizeof(txdata)) == MPSSE_OK)
		{
			printf("Wrote %d bytes over SPI bus.\n", sizeof(txdata));
		}
		else
		{
			printf("Write failed: %s\n", ErrorString());
		}

		Stop();
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	Close();

	return EXIT_SUCCESS;
}
