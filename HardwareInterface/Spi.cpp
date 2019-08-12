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

#define _BV(bit) \
	(1 << (bit))

#define _NBV(bit) \
	(0 << (bit))

#define RESET_BITFLAG_BP      4
#define DO_RESET_BP           3

#define SETUP_BITFLAG_BP      5
#define CKSEL1_BP             5
#define CKSEL0_BP             4
#define REFSEL1_BP            3
#define REFSEL0_BP            2

#define AVERAGING_BITFLAG_BP  6
#define AVGON_BP              4
#define NAVG1_BP              3
#define NAVG0_BP              2
#define NSCAN1_BP             1
#define NSCAN0_BP             0

#define CONVERSION_BITFLAG_BP 7
#define CHSEL3_BP             6
#define CHSEL2_BP             5
#define CHSEL1_BP             4
#define CHSEL0_BP             3
#define SCAN1_BP              2
#define SCAN0_BP              1



#include <iomanip>
namespace
{
	using namespace std;


	const uint32_t NUM_ADCS = 3;
	//max speed is 4.8MHz with clock mode 11
	const uint32_t SPEED = 1000000;

	//Global vars to store configs for all ADC's
	SetupReg_t setupData = {0};
	ConversionReg_t conversionData = {0};

	int fd;

	void adcSPIDataRW(uint8_t channel, unsigned char* buffer, uint32_t len)
	{
		int gpioPin;

		switch (channel)
		{
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



	std::array<uint8_t, 16> adcReadFIFO(uint8_t channel)
	{
		std::array<uint8_t, 16> Output;

		adcWriteConversionReg(channel, &conversionData);
		//512 conversion delay
		std::this_thread::sleep_for(std::chrono::milliseconds(4));

		std::array<uint8_t, 33> Buffer;

		Buffer.fill(0);

		adcSPIDataRW(channel, Buffer.data(), 33);

		for (size_t i = 0; i < 16; ++i)
		{
			int Temp = *(Buffer.data() + 2 * i);

			Temp <<= 4;

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
		unsigned char buf[4] = {0};
		buf[0] = _BV(RESET_BITFLAG_BP)              // Reset register
		       | _BV(DO_RESET_BP)                   // Do reset the device
		    ;
		adcSPIDataRW(channel, buf, 1);


		//Setup reference and timing
		memset(buf, sizeof(buf), 0);
		buf[0] = _BV(SETUP_BITFLAG_BP)               // Setup register 
		       | _BV(CKSEL1_BP)  | _NBV(CKSEL0_BP)   // Internal conversion clock/sampling/acqusition, AIN15 as AIN15, not CNVST
		       | _BV(REFSEL1_BP) | _NBV(REFSEL0_BP)  // Internal voltage reference, keep VRef on after conversion;
		    ;
		adcSPIDataRW(channel, buf, 1);
	 
		//Setup ADC scanning and set to scan all channels
		memset(buf, sizeof(buf), 0);
		buf[0] = _BV(CONVERSION_BITFLAG_BP)                                           // Conversion Register
		       | _BV(CHSEL3_BP) | _BV(CHSEL2_BP) | _BV(CHSEL1_BP)  | _BV(CHSEL0_BP)   // Convert all 16 channels
		       | _NBV(SCAN1_BP) | _NBV(SCAN0_BP)                                      // Scan channels 0 to CHSEL
		    ;
		adcSPIDataRW(channel, buf, 1);


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
	void adcReadFIFOAll(std::array<uint8_t, 48>& Buffer)
	{
		static int Init = adcSetupAll();

		(void)Init;

		for (uint8_t i = 0; i < NUM_ADCS; i++)
		{
			auto Output = adcReadFIFO(i);

			std::memcpy(Buffer.data() + (i * 16), Output.data(), 16);
		}
	}
}
