#pragma once
#include <string>
#include "Input.hpp"

namespace st
{
  struct Success;
  struct Update;

  struct Threshold
  {
    const uint8_t mStart;
    const uint8_t mStop;
    const std::string mLabel;
  };

  class Analog : public st::Input
  {
    public:

      Analog(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand(st::SerialId);

      void IsCorrect(st::Success&);

      void SetCurrentState(uint8_t State);

      uint8_t GetCurrentState() const;

      void Update(const st::Update&);

      const std::vector<st::Threshold>& GetThresholds() const;

    private:

      bool GetIsInCorrectState() const;

      uint8_t GetNewValue(const Threshold& CurrentState);

      const Threshold& GetThreshold(uint8_t Value) const;

      static const std::vector<std::string> mSetWords;

      uint8_t mDesiredValue;

      uint8_t mCurrentState;

      uint64_t mUpdateCount;

      uint64_t mUpdateSum;

      std::vector<Threshold> mThresholds;
  };
}
