#pragma once
#include <array>

namespace st::hw
{
  void adcReadFIFOAll(std::array<uint8_t, 48>& Buffer);
}
