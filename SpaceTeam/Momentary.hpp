#pragma once

#include "Input.hpp"
#include <chrono>

namespace st
{
  struct Success;
  struct Update;

  class Momentary : public st::Input
  {
    public:

      Momentary(const boost::property_tree::ptree& Tree);

      const std::string& GetNewCommand();

      void IsCorrect(st::Success&);

      void SetCurrentState(bool State);

      void Update(const st::Update&);

      const std::string& GetMessage() const;

      bool GetDefaultValue() const;

    private:

      bool IsPressed();

      const bool mDefaultValue;

      const std::string mMessage;

      bool mCurrentState;

      mutable std::chrono::time_point<std::chrono::system_clock> mLastToggle;
  };
}
