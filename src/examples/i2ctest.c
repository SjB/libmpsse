#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define FREQUENCY	200000	// 200KHz

int main(void)
{
	unsigned char txdata[1] = "\xAA";

	if(MPSSE(I2C, FREQUENCY, MSB) == MPSSE_OK)
	{
		printf("I2C bus initialized at %dHz\n", GetClock());
		
		Start();

		if(Write((char *) &txdata, sizeof(txdata)) == MPSSE_OK)
		{
			printf("Wrote %d bytes over I2C bus.\n", sizeof(txdata));
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
