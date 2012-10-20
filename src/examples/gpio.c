#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

int main(void)
{
	struct mpsse_context *io = NULL;
	int i = 0, retval = EXIT_FAILURE;

	io = MPSSE.GPIO.Open();
	
	if(io && io->open)
	{
		for(i=0; i<10; i++)
		{
			MPSSE.GPIO.PinHigh(io, GPIOL0);
			printf("GPIOL0 State: %d\n", MPSSE.GPIO.PinState(io, GPIOL0, -1));
			sleep(1);
			
			MPSSE.GPIO.PinLow(io, GPIOL0);
			printf("GPIOL0 State: %d\n", MPSSE.GPIO.PinState(io, GPIOL0, -1));
			sleep(1);
		}
	
		retval = EXIT_SUCCESS;
	}
	else
	{
		printf("Failed to open MPSSE: %s\n", MPSSE.ErrorString(io));
	}
		
	MPSSE.GPIO.Close(io);

	return retval;
}
