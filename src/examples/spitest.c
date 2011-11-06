/* 
 * Example code to initialize the SPI bus and write one byte.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

#define FREQUENCY	10000000	// 10MHz

int main(void)
{
	if(MPSSE(SPI0, FREQUENCY, MSB) == MPSSE_OK)
	{
		printf("SPI bus initialized at %dHz (mode 0)\n", GetClock());
		
		Start();

		if(Write("\xAA", 1) == MPSSE_OK)
		{
			printf("SPI write successful\n");
		}
		else
		{
			printf("SPI write failed: %s\n", ErrorString());
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
