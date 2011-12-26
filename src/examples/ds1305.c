#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>
#include <unistd.h>

int main(void)
{
	int fd = 0, sec = 0, min = 0, retval = EXIT_FAILURE;
	char *control = NULL, *seconds = NULL, *minutes = NULL;
	
	if((fd = MPSSE(SPI3, ONE_HUNDRED_KHZ, MSB)) != MPSSE_FAIL)
	{
		SetCSIdle(fd, 0);

		printf("%s initialized at %dHz (SPI mode 1)\n", GetDescription(fd), GetClock(fd));
		
		Start(fd);
		Write(fd, "\x0F", 1);
		control = Read(fd, 1);
		Stop(fd);
		
		control[0] &= ~0x80;
		
		Start(fd);
		Write(fd, "\x8F", 1);
		Write(fd, control, 1);
		Stop(fd);

		free(control);

		while(1)
		{
			Start(fd);
			Write(fd, "\x00", 1);
			seconds = Read(fd, 1);
			Stop(fd);

			sec = (((seconds[0] >> 4) * 10) + (seconds[0] & 0x0F));

			Start(fd);
			Write(fd, "\x01", 1);
			minutes = Read(fd, 1);
			Stop(fd);

			min = (((minutes[0] >> 4) * 10) + (minutes[0] & 0x0F));

			printf("%.2d:%.2d\n", min, sec);

			free(minutes);
			free(seconds);

			sleep(1);
		}	
	}
	else
	{
		printf("Failed to initialize MPSSE: %s\n", ErrorString(fd));
	}

	Close(fd);

	return retval;
}
