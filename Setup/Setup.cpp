#include <HardwareInterface/I2c.h>
#include <fmt/format.h>
#include <bitset>
#include <thread>

using namespace std::chrono_literals;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void testPin(size_t Index)
{
  std::bitset<64> Value(0);

  for (int i = 0; i < 6; ++i)
  {
    st::hw::setGPIOVal(Value.to_ulong());

    Value[Index] = !Value[Index];

    std::this_thread::sleep_for(250ms);
  }

  st::hw::setGPIOVal(0);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  fmt::print("Setting up Digital Output Pins\n");

  st::hw::setAllGPIOOutput();

  for (auto i = 0; i < 48; ++i)
  {
    testPin(i);
  }
}
