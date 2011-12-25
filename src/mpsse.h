#ifndef _LIBMPSSE_H_ 
#define _LIBMPSSE_H_

#include <ftdi.h>
#include <stdint.h>

#define MPSSE_OK		0
#define MPSSE_FAIL		-1

#define MSB			0x00
#define LSB			0x08

#define CHUNK_SIZE		65535
#define SPI_TRANSFER_SIZE	(63 * 1024) 
#define I2C_TRANSFER_SIZE	64

#define LATENCY_MS		2
#define TIMEOUT_DIVISOR		1000000
#define USB_TIMEOUT		120000

#define BITMODE_RESET		0
#define BITMODE_MPSSE		2

#define CMD_SIZE		3
#define SS_TX_COUNT		3

#define LOW			0
#define HIGH			1
#define NUM_GPIOL_PINS		4
#define NUM_GPIO_PINS		12

#define MAX_FTDI_CONNECTIONS	512

/* FTDI interfaces */
enum interface
{
	IFACE_ANY	= INTERFACE_ANY,
	IFACE_A 	= INTERFACE_A,
	IFACE_B		= INTERFACE_B,
	IFACE_C		= INTERFACE_C,
	IFACE_D		= INTERFACE_D
};

/* Common clock rates */
enum clock_rates
{
	ONE_HUNDRED_KHZ  = 100000,
	FOUR_HUNDRED_KHZ = 400000,
	ONE_MHZ 	 = 1000000,
	TWO_MHZ		 = 2000000,
	FIVE_MHZ	 = 5000000,
	SIX_MHZ 	 = 6000000,
	TEN_MHZ		 = 1000000,
	TWELVE_MHZ 	 = 12000000,
	THIRTY_MHZ 	 = 30000000,
	SIXTY_MHZ 	 = 60000000
};

/* Supported MPSSE modes */
enum modes
{
	SPI0 = 1,
	SPI1 = 2,
	SPI2 = 3,
	SPI3 = 4,
	I2C  = 5,
	GPIO = 6
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

#define DEFAULT_TRIS            (SK | DO | CS | GPIO0 | GPIO1 | GPIO2 | GPIO3)  /* SK/DO/CS and GPIOs are outputs, DI is an input */
#define DEFAULT_PORT            (SK | CS)       				/* SK and CS are high, all others low */

enum mpsse_commands
{
	INVALID_COMMAND		= 0xAB,
	ENABLE_3_PHASE_CLOCK	= 0x8C,
	DISABLE_3_PHASE_CLOCK	= 0x8D,
	DISABLE_ADAPTIVE_CLOCK	= 0x97,
	TCK_X5			= 0x8A,
	TCK_D5			= 0x8B,
	TOGGLE_CLOCK		= 0x8E
};

enum low_bits_status
{
	STARTED,
	STOPPED
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
	struct ftdi_context ftdi;
	enum modes mode;
	enum low_bits_status status;
	int vid;
	int pid;
	int clock;
	int xsize;
	int index;
	uint8_t tris;
	uint8_t pstart;
	uint8_t pstop;
	uint8_t pidle;
	uint8_t gpioh;
	uint8_t trish;
	uint8_t tx;
	uint8_t rx;
	uint8_t tack;
	uint8_t rack;
};

struct globule mpsse[MAX_FTDI_CONNECTIONS];

int MPSSE(enum modes mode, int freq, int endianess);
int Open(int vid, int pid, int interface, enum modes mode, int freq, int endianess);
void Close(int index);
char *ErrorString(int index);
int SetMode(int index, enum modes mode, int endianess);
int SetClock(int index, uint32_t freq);
int GetClock(int index);
int GetVid(int index);
int GetPid(int index);
char *GetDescription(int index);
int SetLoopback(int index, int enable);
void SetCSIdle(int index, int idle);
int Start(int index);
int Write(int index, char *data, int size);
int Stop(int index);
int GetAck(int index);
void SetAck(int index, int ack);
int PinHigh(int index, int pin);
int PinLow(int index, int pin);

#ifdef SWIGPYTHON

typedef struct swig_string_data
{
        int size;
        char *data;
} swig_string_data;

swig_string_data Read(int index, int size);

#else

char *Read(int index, int size);

#endif

#endif
