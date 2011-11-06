/* 
 * Example code to initialize the SPI bus and write one byte.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define FREQUENCY	200000	// 200KHz

int main(void)
{
	if(MPSSE(I2C, FREQUENCY, MSB) == MPSSE_OK)
	{
		printf("I2C bus initialized at %dHz\n", GetClock());
		
		Start();

		if(Write("\xAA", 1) == MPSSE_OK)
		{
			printf("I2C write successful\n");
		}
		else
		{
			printf("I2C write failed: %s\n", ErrorString());
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
