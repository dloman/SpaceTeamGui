#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <boost/property_tree/ptree.hpp>
#include <variant>
#include <vector>

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  struct Success;

  class Game
  {
    public:

      Game(boost::property_tree::ptree& Tree);

      std::string GetNextInputDisplay();

      void UpdateCurrentState();

      st::Success GetSuccess();

      std::chrono::time_point<std::chrono::system_clock> GetLastResetTime() const;

    private:

      double GetCurrentState(const InputVariant& Input);

      std::vector<st::InputVariant> mInputs;

      std::optional<std::reference_wrapper<InputVariant>> moCurrentActiveVariant;

      std::chrono::time_point<std::chrono::system_clock> mLastResetTime;
  };
}
