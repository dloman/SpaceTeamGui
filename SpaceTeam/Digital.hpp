#pragma once

#include "Input.hpp"

namespace st
{
  struct Success;
  struct Update;

  class Digital : public st::Input
  {
    public:

      Digital(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand(st::SerialId);

      void IsCorrect(st::Success&);

      void SetCurrentState(uint8_t State);

      uint8_t GetCurrentState() const;

      void Update(const st::Update&);

      const std::string& GetOnLabel() const;

      const std::string& GetOffLabel() const;

    private:

      bool IsInCorrectState() const;

      uint8_t mDesiredState;

      uint8_t mCurrentState;

      const std::string mOnLabel;

      const std::string mOffLabel;

      std::vector<uint8_t> mUpdates;
  };
}
