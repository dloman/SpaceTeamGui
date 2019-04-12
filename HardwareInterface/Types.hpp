//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint64_t GetSerialNumber()
{
  uint64_t SerialNumber = 0u;

  std::ifstream Stream("/proc/cpuinfo");

  if (!Stream)
  {
    throw std::runtime_error("unable to open /proc/cpuinfo");
  }

  std::string Word;

  using std::literals;

  while(!Stream.eof())
  {
    if (Stream >> Line; Stream == "Serial"s)
    {
      Stream >> Line; // :

      Stream >> Line; // Serial #

      Stream >> std::hex >> SerialNumber;
    }
  }

  return SerialNumber;
}

const static uint64_t gSerialNumber = GetSerialNumber();

enum class eDeviceID : uint8_t
{
  eDigital,
  eAnalog
};

//Mike To Dan
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct DigitalDataOut
{
   //8 bytes (unique to PI)
   uint64_t mPiSerial;

   //4 bytes (unique to specific chip on swiss's board, always zero for this board)
   eDeviceID mDeviceID;

   //5 bytes (normal logic state i.e. 1 = High/Unconnected, read from all pins (even if set as output))
   uint8_t mDataRead[5];

   //5 bytes (bitmask of digital pin directions, 1 = input, 0 = output)
   uint8_t mDataDirMask[5];

   //5 bytes (output state of digital pins modulo the above dataDirMaks, i.e. setting state of input pin will do nothing)
   uint8_t mDataWrite[5];
};

//Dan To Mike
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct DigitalDataIn
{
   //8 Bytes (serial of Pi you want to modify, used to gaurd against random data on port)
   uint64_t mPiSerial;

   //4 bytes (deviceID )
   eDeviceID mDeviceID;

   //5 bytes (used to set bitmask of digital pins, 1 = input, 0 = output )
   uint8_t mDataDirMask[5];

   //5 bytes (output state of digital pins modulo the above dataDirMaks, i.e. setting state of input pin will do nothing)
   uint8_t mDataWrite[5];
}

//sent from driver
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct AnalogDataOut
{
   //8 bytes (unique to PI)
   uint64_t mPiSerial;

   //4 bytes (unique to specific chip on swiss's board, will be 1 for this board)
   eDeviceID mDeviceID;

   //48 bytes (raw ADC samples for each channel ADC1 - ADC48)
   uint8_t mDataRead[48];
}
