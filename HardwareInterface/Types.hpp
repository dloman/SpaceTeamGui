#pragma once

#include <fstream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
uint64_t GetSerialNumber()
{
  uint64_t SerialNumber = 0u;

  std::ifstream Stream("/proc/cpuinfo");

  if (!Stream)
  {
    throw std::runtime_error("unable to open /proc/cpuinfo");
  }

  std::string Word;

  using namespace std::literals;

  while(!Stream.eof())
  {
    if (Stream >> Word; Word == "Serial"s)
    {
      Stream >> Word; // :

      Stream >> std::hex >> SerialNumber;
    }
  }

  return SerialNumber;
}

const static uint64_t gSerialNumber = GetSerialNumber();

enum class eDeviceID : uint8_t
{
  eDigital = 0xFF,
  eAnalog = 0x00
};
