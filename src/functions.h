#ifndef _FUNCTIONS_H_ 
#define _FUNCTIONS_H_

#include <ftdi.h>
#include <stdint.h>
#include "mpsse.h"
#include "swig.h"

struct mpsse_context *EasyOpen(enum modes mode, int freq, int endianess);
struct mpsse_context *Open(int vid, int pid, enum modes mode, int freq, int endianess, int interface, const char *description, const char *serial);
void Close(struct mpsse_context *mpsse);
char *ErrorString(struct mpsse_context *mpsse);
int SetMode(struct mpsse_context *mpsse, int endianess);
int SetClock(struct mpsse_context *mpsse, uint32_t freq);
int GetClock(struct mpsse_context *mpsse);
int GetVid(struct mpsse_context *mpsse);
int GetPid(struct mpsse_context *mpsse);
char *GetDescription(struct mpsse_context *mpsse);
int SetLoopback(struct mpsse_context *mpsse, int enable);
void SetCSIdle(struct mpsse_context *mpsse, int idle);
int Start(struct mpsse_context *mpsse);
int Write(struct mpsse_context *mpsse, char *data, int size);
int MCUWrite(struct mpsse_context *mpsse, char *data, int size, int address);
int Stop(struct mpsse_context *mpsse);
int GetAck(struct mpsse_context *mpsse);
void SetAck(struct mpsse_context *mpsse, int ack);
void SendAcks(struct mpsse_context *mpsse);
void SendNacks(struct mpsse_context *mpsse);
int PinHigh(struct mpsse_context *mpsse, int pin);
int PinLow(struct mpsse_context *mpsse, int pin);
int ReadPins(struct mpsse_context *mpsse);
int PinState(struct mpsse_context *mpsse, int pin, int state);
int ClockUntilHigh(struct mpsse_context *mpsse);
int ClockUntilLow(struct mpsse_context *mpsse);
int ToggleClock(struct mpsse_context *mpsse, int count);
int ToggleClockX8(struct mpsse_context *mpsse, int count, int gpio);
int Tristate(struct mpsse_context *mpsse);
int Version(void);

#ifdef SWIGPYTHON
swig_string_data Read(struct mpsse_context *mpsse, int size);
swig_string_data MCURead(struct mpsse_context *mpsse, int size, int address);
swig_string_data Transfer(struct mpsse_context *mpsse, char *data, int size);
#else
char *Read(struct mpsse_context *mpsse, int size);
char *MCURead(struct mpsse_context *mpsse, int size, int address);
char *Transfer(struct mpsse_context *mpsse, char *data, int size);
#endif

#endif
