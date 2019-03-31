#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <variant>
#include <vector>

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  class Game
  {
    public:

      Game();

      std::string GetNextInputDisplay();

    private:

      std::vector<st::InputVariant> mInputs;
  };
}
