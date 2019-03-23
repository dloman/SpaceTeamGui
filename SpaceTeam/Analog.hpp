#pragma once
#include <string>
#include "Input.hpp"

namespace st
{
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

      std::string GetNewCommand(double CurrentState);

      bool IsCommandCompleted(double CurrentState);

    private:

      double GetNewValue(const Threshold& CurrentState);

      const Threshold& GetThreshold(double Value) const;

      static const std::vector<std::string> mSetWords;

      double mDesiredValue;

      std::vector<Threshold> mThresholds;
  };
}
