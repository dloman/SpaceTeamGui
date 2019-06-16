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

      const std::string& GetNewCommand(st::SerialId);

      void IsCorrect(st::Success&);

      void SetCurrentState(bool State);

      void Update(const st::Update&);

      const std::string& GetMessage() const;

      bool GetDefaultValue() const;

    private:

      bool WasPressed();

      const bool mDefaultValue;

      const std::string mMessage;

      bool mCurrentState;

      std::vector<bool> mUpdates;

      mutable std::chrono::time_point<std::chrono::system_clock> mLastToggle;
  };
}
