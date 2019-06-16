#pragma once
#include <SpaceTeam/Analog.hpp>
#include <SpaceTeam/Digital.hpp>
#include <SpaceTeam/Momentary.hpp>
#include <SpaceTeam/Output.hpp>
#include <SpaceTeam/Id.hpp>
#include <boost/property_tree/ptree.hpp>
#include <variant>
#include <vector>
#include <unordered_set>

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  constexpr int StartingScore = 100;

  struct Success;
  class UpdateVec;

  class Game
  {
    public:

      Game(boost::property_tree::ptree& Tree);

      std::string GetNextInputDisplay(st::SerialId);

      void UpdateCurrentState(st::UpdateVec& Updates);

      st::Success GetSuccess();

      std::chrono::time_point<std::chrono::system_clock> GetLastResetTime(st::SerialId) const;

      int GetCurrentScore() const;

      void SetCurrentScore(int);

      int GetCurrentRound() const;

      void SetCurrentRound(int);

      void Success(bool Success, st::SerialId Serial);

      void GetNextRoundInputs(
        const std::vector<st::SerialId>& ActivePanelSerialNumbers);

      st::HardwareDirection GetHardwareDirection(st::SerialId PiSerial) const;

      st::HardwareValue GetHardwareValue(st::SerialId PiSerial) const;

      const std::unordered_set<st::SerialId>& GetPiSerials() const;

      void UpdateOutputs();

      const std::vector<st::InputVariant>& GetInputs() const;

      const std::vector<st::Output>& GetOutputs() const;

      std::vector<std::reference_wrapper<st::InputVariant>> GetCurrentRoundInputs() const;

    private:

      size_t GetRoundSizePerPanel();

      double GetCurrentState(const InputVariant& Input);

      std::vector<st::InputVariant> mInputs;

      std::unordered_map<st::SerialId, std::optional<std::reference_wrapper<InputVariant>>> mCurrentActiveVariants;

      std::vector<st::Output> mOutputs;

      const std::unordered_set<st::SerialId> mPiSerials;

      std::vector<std::reference_wrapper<InputVariant>> mCurrentRoundInputs;

      std::unordered_map<st::SerialId, std::chrono::time_point<std::chrono::system_clock>> mLastResetTime;

      const std::unordered_map<st::SerialId, int> mStats;

      static int mCurrentScore;

      static int mCurrentRound;
  };
}
