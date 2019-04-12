#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <bitset>
#include <chrono>
#ifdef ENABLE_HARDWARE
#include <wiringPiI2C.h>


#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_IP0  0x00
#define COMMAND_IP1  0x01
#define COMMAND_IP2  0x02
#define COMMAND_IP3  0x03
#define COMMAND_IP4  0x04

#define COMMAND_OP0  0x08
#define COMMAND_OP1  0x09
#define COMMAND_OP2  0x0A
#define COMMAND_OP3  0x0B
#define COMMAND_OP4  0x0C

#define COMMAND_PI0  0x10
#define COMMAND_PI1  0x11
#define COMMAND_PI2  0x12
#define COMMAND_PI3  0x13
#define COMMAND_PI4  0x14

#define COMMAND_IOC0 0x18
#define COMMAND_IOC1 0x19
#define COMMAND_IOC2 0x1A
#define COMMAND_IOC3 0x1B
#define COMMAND_IOC4 0x1C

#define COMMAND_MSK0 0x20
#define COMMAND_MSK1 0x21
#define COMMAND_MSK2 0x22
#define COMMAND_MSK3 0x23
#define COMMAND_MSK4 0x24

#define COMMAND_REG_AUTO_INC  0x80
#define COMMAND_REG           0x00

#define DIR_ALL_INPUT         0x000000FFFFFFFFFF
#define DIR_ALL_OUTPUT        0x0000000000000000

#define POLARITY_ALL_NORMAL   0x0000000000000000
#define POLARITY_ALL_INVERTED 0x000000FFFFFFFFFF

#define VALUE_ALL_OFF         0x0000000000000000
#define VALUE_ALL_ON          0x000000FFFFFFFFFF

namespace
{
	int write8(uint8_t reg, uint8_t val){
		 int result = wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | reg, val);
		 return result;
	}

  int GetFileDescriptor()
  {
    static int FileDescriptor = wiringPiI2CSetup(0x20);

    return FileDescriptor;
  }
}

namespace st::hw
{
int setGPIODir(uint64_t dir)
{
   int result = wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC0, (dir >> 0 & 0xFF));
   result |= wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC1, (dir >> 8 & 0xFF));
   result |= wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC2, (dir >> 16 & 0xFF));
   result |= wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC3, (dir >> 24 & 0xFF));
   result |= wiringPiI2CWriteReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC4, (dir >> 32 & 0xFF));
   return result;
}

int setGPIOVal(uint64_t val)
{
   int result = write8(GetFileDescriptor(), COMMAND_REG | COMMAND_OP0, (val >> 0) & 0xFF );
   cout << "MSB: " << hex << (unsigned int)((val >> 0) & 0xFF);
   result |= write8(GetFileDescriptor(), COMMAND_REG | COMMAND_OP1, (val >> 8) & 0xFF );
   cout << " " << (unsigned int)((val >> 8) & 0xFF);
   result |= write8(GetFileDescriptor(), COMMAND_REG | COMMAND_OP2, (val >> 16) & 0xFF );
   cout << " " << (unsigned int)((val >> 16) & 0xFF);
   result |= write8(GetFileDescriptor(), COMMAND_REG | COMMAND_OP3, (val >> 24) & 0xFF );
   cout << " " << (unsigned int)((val >> 24) & 0xFF);
   result |= write8(GetFileDescriptor(), COMMAND_REG | COMMAND_OP4, (val >> 32) & 0xFF );
   cout << " " << (unsigned int)((val >> 32) & 0xFF);
   cout << " LSB" << endl;
   return result;
}

int setAllGPIOInput()
{
  setGPIODir(GetFileDescriptor(), DIR_ALL_INPUT);
}

int setAllGPIOOutput()
{
  setGPIODir(GetFileDescriptor(), DIR_ALL_OUTPUT);
}

uint64_t getGPIOVal()
{
   uint64_t value = 0;
   uint64_t result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IP0);
   value = result;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IP1);
   value |= result << 8;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IP2);
   value |= result << 16;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IP3);
   value |= result << 24;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IP4);
   value |= result << 32;
   return value;
}

uint64_t getGPIODir()
{
   uint64_t value = 0;
   uint64_t result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC0);
   value = result;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC1);
   value |= result << 8;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC2);
   value |= result << 16;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC3);
   value |= result << 24;
   result = wiringPiI2CReadReg8(GetFileDescriptor(), COMMAND_REG | COMMAND_IOC4);
   value |= result << 32;
   return value;
}
#endif
