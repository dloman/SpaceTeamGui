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

namespace dl::tcp
{
  class Session;
}

namespace st
{
  using InputVariant = std::variant<st::Analog, st::Digital, st::Momentary>;

  constexpr int StartingScore = 100;

  struct Success;
  class Panel;
  class UpdateVec;

  class Game
  {
    public:

      Game(boost::property_tree::ptree& Tree, const std::vector<std::unique_ptr<st::Panel>>& Panels);

      void UpdateCurrentState(st::UpdateVec& Updates);

      int GetCurrentScore() const;

      void SetCurrentScore(int);

      int GetCurrentRound() const;

      void SetCurrentRound(int);

      void SendNewRound();

      const std::vector<st::Output>& GetOutputs() const;

      const std::unordered_set<st::SerialId>& GetPiSerials() const;

      const std::vector<st::InputVariant>& GetInputs() const;

    private:

      void GetInitialInputDisplays();

      std::string GetNextInputDisplay(st::SerialId);

      std::chrono::time_point<std::chrono::system_clock> GetLastResetTime(st::SerialId) const;

      void GetNextRoundInputs();

      st::HardwareDirection GetHardwareDirection(st::SerialId PiSerial) const;

      st::HardwareValue GetHardwareValue(st::SerialId PiSerial) const;

      void UpdateOutputs();

      std::vector<std::reference_wrapper<st::InputVariant>> GetCurrentRoundInputs() const;

      void GetInitialInputDisplay(st::SerialId);

      st::Success GetSuccess();

      void UpdateScore(bool Success);

      size_t GetRoundSizePerPanel();

      double GetCurrentState(const InputVariant& Input);

      void SendGpioDirections();

      void SendGpioValues();

      void SendReset(
        st::SerialId SerialId,
        dl::tcp::Session& Session,
        std::string Value = "");

      std::vector<st::InputVariant> mInputs;

      std::unordered_map<st::SerialId, std::optional<std::reference_wrapper<InputVariant>>> mCurrentActiveVariants;

      std::vector<st::Output> mOutputs;

      std::unordered_map<st::SerialId, std::shared_ptr<dl::tcp::Session>> mSerialToSession;

      const std::unordered_set<st::SerialId> mSerials;

      std::vector<std::reference_wrapper<InputVariant>> mCurrentRoundInputs;

      std::unordered_map<st::SerialId, std::chrono::time_point<std::chrono::system_clock>> mLastResetTime;

      const std::unordered_map<st::SerialId, int> mStats;

      static int mCurrentScore;

      static int mCurrentRound;
  };
}
