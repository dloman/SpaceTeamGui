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

      std::string GetNewCommand();

      void IsCorrect(st::Success&);

      void SetCurrentState(bool State);

      bool GetCurrentState() const;

      void Update(const st::Update&);

      const std::string& GetOnLabel() const;

      const std::string& GetOffLabel() const;

    private:

      bool IsInCorrectState() const;

      bool mDesiredState;

      bool mCurrentState;

      const std::string mOnLabel;

      const std::string mOffLabel;
  };
}
