#pragma once

#include "Input.hpp"
#include <chrono>

namespace st
{
  struct Success;

  class Momentary : public st::Input
  {
    public:

      Momentary(const boost::property_tree::ptree& Tree);

      const std::string& GetNewCommand();

      void IsCorrect(st::Success&);

      void SetCurrentState(bool State);

    private:

      bool IsPressed();

      const bool mDefaultValue;

      const std::string mMessage;

      bool mCurrentState;

      mutable std::chrono::time_point<std::chrono::system_clock> mLastToggle;
  };
}
