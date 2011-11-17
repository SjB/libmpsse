#ifndef _SUPPORT_H_
#define _SUPPORT_H_

int raw_write(unsigned char *buf, int size);
int raw_read(unsigned char *buf, int size);
void set_timeouts(int timeout);
uint16_t freq2div(uint32_t system_clock, uint32_t freq);
uint32_t div2freq(uint32_t system_clock, uint16_t div);
unsigned char *build_block_buffer(uint8_t cmd, unsigned char *data, int size, int *buf_size);
int gpio_write(int pin, int direction);

#endif
