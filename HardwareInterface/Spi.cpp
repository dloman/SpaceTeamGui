#include <iostream>
#include <errno.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <chrono>
#include <thread>

#include <iomanip>
namespace
{
using namespace std;


#define CONVERSION_REG_ADDR  0x80
typedef union {
   uint8_t data;
   struct {
      volatile uint8_t bit0 : 1;
      volatile uint8_t SCAN : 2;
      volatile uint8_t CHSEL : 4;
      volatile uint8_t bit7 : 1;
   }bits;
} ConversionReg_t;

#define SETUP_REG_ADDR  0x40
typedef union {
   uint8_t data;
   struct {
      volatile uint8_t bit0 : 1;
      volatile uint8_t bit1 : 1;
      volatile uint8_t REFSEL : 2;
      volatile uint8_t CKSEL : 2;
      volatile uint8_t bit6 : 1;
      volatile uint8_t bit7 : 1;
   }bits;
} SetupReg_t;

#define AVERAGING_REG_ADDR  0x20
typedef union {
   uint8_t data;
   struct {
      volatile uint8_t NSCAN : 2;
      volatile uint8_t NAVG : 2;
      volatile uint8_t AVGON : 1;
      volatile uint8_t bit5 : 1;
      volatile uint8_t bit6 : 1;
      volatile uint8_t bit7 : 1;
   }bits;
} AveragingReg_t;

#define RESET_REG_ADDR  0x10
typedef union {
   uint8_t data;
   struct {
      volatile uint8_t bit0 : 1;
      volatile uint8_t bit1 : 1;
      volatile uint8_t bit2 : 1;
      volatile uint8_t RESET : 1;
      volatile uint8_t bit4 : 1;
      volatile uint8_t bit5 : 1;
      volatile uint8_t bit6 : 1;
      volatile uint8_t bit7 : 1;
   }bits;
} ResetReg_t;

const uint32_t NUM_ADCS = 3;
//max speed is 4.8MHz with clock mode 11
const uint32_t SPEED = 1000000;

//Global vars to store configs for all ADC's
SetupReg_t setupData = {0};
ConversionReg_t conversionData = {0};

int fd;

void adcSPIDataRW(uint8_t channel, unsigned char* buffer, uint32_t len){
   int gpioPin;
   switch(channel){
      case 0:
      {
         gpioPin = 24;
         break;
      }
      case 1:
      {
         gpioPin = 28;
         break;
      }
      case 2:
      {
         gpioPin = 29;
         break;
      }
   }
   digitalWrite(gpioPin, LOW);
   usleep(10);
   wiringPiSPIDataRW(0, buffer, len);
   usleep(10);
   digitalWrite(gpioPin, HIGH);
}

void adcWriteConversionReg(uint8_t channel, ConversionReg_t* data)
{
   unsigned char buf[4];
   buf[1] = buf[2] = 0;
   buf[0] = data->data;
   adcSPIDataRW(channel, buf, 3);
}

void adcWriteSetupReg(uint8_t channel, SetupReg_t* data)
{
   unsigned char buf[2];
   buf[0] = data->data;
   adcSPIDataRW(channel, buf, 1);
}

/*
void adcWriteAveragingReg(uint8_t channel, AveragingReg_t* data)
{
   unsigned char buf[2];
   buf[0] = data->data;
   adcSPIDataRW(channel, buf, 1);
}*/

void adcWriteResetReg(uint8_t channel, ResetReg_t* data)
{
   unsigned char buf[2];
   buf[0] = data->data;
   adcSPIDataRW(channel, buf, 1);
}

std::array<uint8_t, 8> adcReadFIFO(uint8_t channel)
{
   std::array<uint8_t, 8> Output;

   adcWriteConversionReg(channel, &conversionData);
   //512 conversion delay
   std::this_thread::sleep_for(std::chrono::milliseconds(4));

   std::array<uint8_t, 17> Buffer;

   Buffer.fill(0);

   adcSPIDataRW(channel, Buffer.data(), 17);

   for (size_t i = 0; i < 8; ++i)
   {
     int Temp = *(Buffer.data() + 2*i);

     Temp<<=4;

     Output[i] = Temp;
   }

   return Output;
}



void adcSetup(uint8_t channel)
{
   //CPOL = CPHA = 0
   int spiMode = 0;

   //initialize pins
   wiringPiSetup();
   pinMode(24, OUTPUT);
   pinMode(29, OUTPUT);
   pinMode(28, OUTPUT);
   digitalWrite(24, HIGH);
   digitalWrite(29, HIGH);
   digitalWrite(28, HIGH);

   //Initialize descriptor
   fd = wiringPiSPISetup(0, SPEED);

   ioctl(fd, SPI_IOC_WR_MODE, &spiMode);

   //Reset ADC
   ResetReg_t resetData = {0};
   resetData.data = RESET_REG_ADDR;
   resetData.bits.RESET = 1;
   adcWriteResetReg(channel, &resetData);

   //Setup reference and timing
   setupData.data = SETUP_REG_ADDR;
   setupData.bits.CKSEL = 2;  //Internally timed, internally triggered, AIN15/7
   setupData.bits.REFSEL = 2; //Reference always on
   adcWriteSetupReg(channel, &setupData);

   //Setup ADC scanning and set to scan all channels
   conversionData.data = CONVERSION_REG_ADDR;
   conversionData.bits.SCAN = 0; //Scans 0 through N channels
   conversionData.bits.CHSEL = 0x0F; //N = ANIN15
   adcWriteConversionReg(channel, &conversionData);

}

int adcSetupAll()
{
   for (uint8_t i = 0; i < NUM_ADCS; i++)
   {
      adcSetup(i);
   }

   return 1;
}
}

namespace st::hw
{
  //Needs 48 bytes of buffer
  void adcReadFIFOAll(std::array<uint8_t, 24>& Buffer)
  {
    static int Init = adcSetupAll();

    (void)Init;

     for(uint8_t i = 0; i < NUM_ADCS; i++)
     {
        auto Output = adcReadFIFO(i);

        std::memcpy(Buffer.data() + (i * 8), Output.data(), 8);
     }
  }
}
