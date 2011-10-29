#ifndef _LIBMPSSE_H_ 
#define _LIBMPSSE_H_

#include <ftdi.h>
#include <stdint.h>
#include <stdarg.h>

#define MPSSE_OK		0
#define MPSSE_FAIL		1

#define MSB			0x00
#define LSB			0x08

#define TRANSFER_SIZE		((64 * 1024) - 1)

#define LATENCY_MS		2
#define TIMEOUT_MS		10000
#define TIMEOUT_DIVISOR		1000000

#define SIX_MHZ			6000000
#define TWELVE_MHZ		12000000
#define SIXTY_MHZ		60000000

#define BITMODE_RESET		0
#define BITMODE_MPSSE		2

#define CMD_SIZE		3

/* Supported MPSSE modes */
enum modes
{
	SPI0,
	SPI1,
	SPI2,
	SPI3,
	I2C
};

enum pins
{
	SK	= 1,
	DO	= 2,
	DI	= 4,
	CS	= 8 ,
	GPIO0	= 16,
	GPIO1	= 32,
	GPIO2	= 64,
	GPIO3	= 128
};

#define DEFAULT_TRIS            (SK | DO | CS)  /* SK/DO/CS are outputs, all others inputs */
#define DEFAULT_PORT            (SK | CS)       /* SK and CS are high, all others low */

enum mpsse_commands
{
	INVALID_COMMAND		= 0xAB,
	ENABLE_3_PHASE_CLOCK	= 0x8C,
	DISABLE_3_PHASE_CLOCK	= 0x8D,
	DISABLE_ADAPTIVE_CLOCK	= 0x97,
	TCK_X5			= 0x8A,
	TCK_D5			= 0x8B,
	TOGGLE_CLOCK		= 0x8E,
	WRITE_DATA_MSB		= 0x10,
	READ_DATA_MSB		= 0x20,
	WRITE_DATA_LSB		= 0x19,
	READ_DATA_LSB		= 0x28
};

struct vid_pid
{
	int vid;
	int pid;
	char *description;
};

struct globule
{
	char *description;
	enum modes mode;
	struct ftdi_context ftdi;
	int vid;
	int pid;
	int clock;
	char tris;
	char pstart;
	char pstop;
	char pidle;
	uint8_t tx;
	uint8_t rx;
} mpsse;

int MPSSE(enum modes mode, int freq, int endianess);
int Open(int vid, int pid, enum modes mode, int freq, int endianess);
void Close();
char *ErrorString(void);
void SetTimeouts(int timeout);
int SetMode(enum modes mode, int endianess);
uint32_t SetClock(uint32_t freq);
int SetLoopback(int enable);
int Start(void);
int Write(char *data, int size);
int Stop(void);

#ifdef SWIGPYTHON

typedef struct swig_string_data
{
        int size;
        char *data;
} swig_string_data;

swig_string_data Read(int size);

#else

char *Read(int size);

#endif

#endif
