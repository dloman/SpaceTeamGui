#pragma once

#include "Input.hpp"

namespace st
{
  class Momentary : public st::Input
  {
    public:

      Momentary(const boost::property_tree::ptree& Tree);

      const std::string& GetNewCommand();

      bool IsCommandCompleted() const;

      void SetCurrentState(bool State);

    private:

      const bool mDefaultValue;

      const std::string mMessage;

      bool mCurrentState;
  };
}
