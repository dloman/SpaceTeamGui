#pragma once
#include <string>
#include "Input.hpp"

namespace st
{
  struct Success;
  struct Update;

  struct Threshold
  {
    const double mStart;
    const double mStop;
    const std::string mLabel;
  };

  class Analog : public st::Input
  {
    public:

      Analog(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand();

      void IsCorrect(st::Success&);

      void SetCurrentState(double State);

      void Update(const st::Update&);

    private:

      bool IsInCorrectState() const;

      double GetNewValue(const Threshold& CurrentState);

      const Threshold& GetThreshold(double Value) const;

      static const std::vector<std::string> mSetWords;

      double mDesiredValue;

      double mCurrentState;

      std::vector<Threshold> mThresholds;
  };
}
