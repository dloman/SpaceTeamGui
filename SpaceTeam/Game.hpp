#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <boost/property_tree/ptree.hpp>
#include <variant>
#include <vector>
#include <unordered_set>

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  struct Success;
  class UpdateVec;

  class Game
  {
    public:

      Game(boost::property_tree::ptree& Tree);

      std::string GetNextInputDisplay();

      void UpdateCurrentState(st::UpdateVec& Updates);

      st::Success GetSuccess();

      std::chrono::time_point<std::chrono::system_clock> GetLastResetTime() const;

      int GetCurrentScore() const;

      void SetCurrentScore(int);

      int GetCurrentRound() const;

      void SetCurrentRound(int);

      void Success(bool Success);

      void GetNextRoundInputs();

    private:

      size_t GetRoundInputsSize() const;

      double GetCurrentState(const InputVariant& Input);

      std::vector<st::InputVariant> mInputs;

      std::optional<std::reference_wrapper<InputVariant>> moCurrentActiveVariant;

      std::vector<std::reference_wrapper<InputVariant>> mCurrentRoundInputs;

      std::chrono::time_point<std::chrono::system_clock> mLastResetTime;

      static int mCurrentScore;

      static int mCurrentRound;
  };
}
