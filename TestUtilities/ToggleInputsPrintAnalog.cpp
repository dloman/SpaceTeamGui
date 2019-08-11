#include <HardwareInterface/I2c.h>
#include <HardwareInterface/Spi.h>
#include <HardwareInterface/Types.hpp>

#include <boost/property_tree/json_parser.hpp>

#include <fmt/format.h>

#include <bitset>
#include <iostream>
#include <optional>
#include <thread>

std::string gPiSerial;

using namespace std::literals::chrono_literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void TogglePins()
{
  st::hw::setGPIOVal(std::numeric_limits<uint64_t>::max());

  std::this_thread::sleep_for(250ms);

  st::hw::setGPIOVal(0);

  std::this_thread::sleep_for(250ms);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PrintAnalogState()
{
  std::array<uint8_t, 24> Analog;

  st::hw::adcReadFIFOAll(Analog);

  for (const auto& Val : Analog)
  {
    fmt::print("{:x} ", Val);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  while (true)
  {
    PrintAnalogState();

    TogglePins();
  }
}
