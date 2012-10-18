#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>

int main(void)
{
	struct mpsse_context *io = NULL;
	int i = 0, retval = EXIT_FAILURE;

	io = mpsse_easy_open(BITBANG, 0, 0);

	if(io && io->open)
	{
		for(i=0; i<10; i++)
		{
			mpsse_pin_high(io, 0);
			printf("Pin 0 is: %d\n", mpsse_pin_state(io, 0, -1));
			sleep(1);
			
			mpsse_pin_low(io, 0);
			printf("Pin 0 is: %d\n", mpsse_pin_state(io, 0, -1));
			sleep(1);
		}

		retval = EXIT_SUCCESS;
	}
	else
	{
		printf("Failed to open MPSSE: %s\n", mpsse_error_string(io));
	}

	mpsse_close(io);

	return retval;
}
