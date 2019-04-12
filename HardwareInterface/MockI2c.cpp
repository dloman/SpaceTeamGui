#include "I2c.h"

namespace st::hw
{
  int setGPIODir(uint64_t dir)
  {
    return 1;
  }

  int setGPIOVal(uint64_t val)
  {
    return 1;
  }

  int setAllGPIOInput()
  {
    return 1;
  }

  int setAllGPIOOutput()
  {
    return 1;
  }

  uint64_t getGPIOVal()
  {
    return 0xFFFFFFFF;
  }

  uint64_t getGPIODir()
  {
    return 0xFFFFFFFF;
  }
}
