#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>
#include <unistd.h>

int main(void)
{
	int sec = 0, min = 0, retval = EXIT_FAILURE;
	char *control = NULL, *seconds = NULL, *minutes = NULL;
	
	if(MPSSE(SPI3, ONE_HUNDRED_KHZ, MSB) == MPSSE_OK)
	{
		SetCSIdle(0);

		printf("%s initialized at %dHz (SPI mode 1)\n", GetDescription(), GetClock());
		
		Start();
		Write("\x0F", 1);
		control = Read(1);
		Stop();
		
		control[0] &= ~0x80;
		
		Start();
		Write("\x8F", 1);
		Write(control, 1);
		Stop();

		free(control);

		while(1)
		{
			Start();
			Write("\x00", 1);
			seconds = Read(1);
			Stop();

			sec = (((seconds[0] >> 4) * 10) + (seconds[0] & 0x0F));

			Start();
			Write("\x01", 1);
			minutes = Read(1);
			Stop();

			min = (((minutes[0] >> 4) * 10) + (minutes[0] & 0x0F));

			printf("%.2d:%.2d\n", min, sec);

			free(minutes);
			free(seconds);

			sleep(1);
		}	
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString());
	}

	Close();

	return retval;
}
