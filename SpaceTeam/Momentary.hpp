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

      void SetCurrentState(uint8_t State);

      void Update(const st::Update&);

      const std::string& GetMessage() const;

      uint8_t GetDefaultValue() const;

    private:

      bool WasPressed();

      const uint8_t mDefaultValue;

      const std::string mMessage;

      uint8_t mCurrentState;

      std::vector<uint8_t> mUpdates;

      mutable std::chrono::time_point<std::chrono::system_clock> mLastToggle;
  };
}
