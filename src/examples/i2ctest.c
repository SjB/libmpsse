#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define FREQUENCY	200000	// 200KHz

int main(void)
{
	unsigned char txdata[1] = "\xAA";

	if(MPSSE(I2C, FREQUENCY, MSB) == MPSSE_OK)
	{
		printf("Requested frequency: %d\nActual frequency: %d\n", FREQUENCY, mpsse.clock);
		
		Start();
		Write((char *) &txdata, 1);
		Stop();
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	Close();

	return EXIT_SUCCESS;
}
