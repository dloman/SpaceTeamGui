#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <variant>
#include <vector>

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  struct Success;

  class Game
  {
    public:

      Game();

      std::string GetNextInputDisplay();

      void UpdateCurrentState();

      st::Success GetSuccess();

    private:

      double GetCurrentState(const InputVariant& Input);

      std::vector<st::InputVariant> mInputs;

      std::optional<std::reference_wrapper<InputVariant>> moCurrentActiveVariant;
  };
}
