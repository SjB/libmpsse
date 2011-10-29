CC=gcc
SWIG=swig
PYDEV=/usr/include/python2.6/
INC=-I$(PYDEV)
LDFLAGS=-lftdi
CFLAGS=-Wall
TARGET=mpsse

$(TARGET): mpsse.o
	ld -shared $(TARGET).o support.o -o $(TARGET).so $(LDFLAGS)

spitest: mpsse.o
	$(CC) $(CFLAGS) $(LDFLAGS) mpsse.o support.o spitest.c -o spitest

i2ctest: mpsse.o
	$(CC) $(CFLAGS) $(LDFLAGS) mpsse.o support.o i2ctest.c -o i2ctest

mpsse.o: support.o
	$(CC) $(CFLAGS) $(LDFLAGS) -c mpsse.c

support.o:
	$(CC) $(CFLAGS) $(LDFLAGS) -c support.c

swig:
	$(CC) $(CFLAGS) $(LDFLAGS) -DSWIGPYTHON -c support.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DSWIGPYTHON -c mpsse.c
	$(SWIG) -python $(TARGET).i
	$(CC) $(CFLAGS) -c  $(TARGET)_wrap.c $(LDFLAGS) $(INC)
	ld -shared $(TARGET)_wrap.o mpsse.o support.o -o _$(TARGET).so $(LDFLAGS) $(INC)

clean:
	rm -f spitest i2ctest *.o *.so $(TARGET).py* $(TARGET)_wrap.c
