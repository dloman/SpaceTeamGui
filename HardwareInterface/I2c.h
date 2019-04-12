#pragma once

#include <cstdint>

namespace st::hw
{
  int setGPIODir(uint64_t dir);

  int setGPIOVal(uint64_t val);

  int setAllGPIOInput();

  int setAllGPIOOutput();

  uint64_t getGPIOVal();

  uint64_t getGPIODir();
}
