#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <SpaceTeam/Output.hpp>
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

      std::unordered_set<uint64_t> GetNextRoundInputs();

      void SetNextRoundInputs(std::unordered_set<uint64_t>& Indecies);

      uint64_t GetHardwareDirection(uint64_t PiSerial) const;

      uint64_t GetHardwareValue(uint64_t PiSerial) const;

      const std::unordered_set<uint64_t>& GetPiSerials() const;

      void UpdateOutputs();

    private:

      double GetCurrentState(const InputVariant& Input);

      std::vector<st::InputVariant> mInputs;

      std::optional<std::reference_wrapper<InputVariant>> moCurrentActiveVariant;

      std::vector<st::Output> mOutputs;

      const std::unordered_set<uint64_t> mPiSerials;

      std::vector<std::reference_wrapper<InputVariant>> mCurrentRoundInputs;

      std::chrono::time_point<std::chrono::system_clock> mLastResetTime;

      static int mCurrentScore;

      static int mCurrentRound;
  };
}
