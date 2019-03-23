#pragma once

#include "Input.hpp"

namespace st
{
  class Digital : public st::Input
  {
    public:

      Digital(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand(double CurrentState);

      bool IsCommandCompleted(bool State);

    private:

      bool mDesiredState;

      const std::string mOnLabel;

      const std::string mOffLabel;
  };
}
