#pragma once

#include "Input.hpp"

namespace st
{
  class Digital : public st::Input
  {
    public:

      Digital(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand();

      bool IsCommandCompleted() const;

      void SetCurrentState(bool State);

    private:

      bool mDesiredState;

      bool mCurrentState;

      const std::string mOnLabel;

      const std::string mOffLabel;
  };
}
