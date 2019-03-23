#pragma once

#include "Input.hpp"

namespace st
{
  class Momentary : public st::Input
  {
    public:

      Momentary(const boost::property_tree::ptree& Tree);

      std::string GetNewCommand();

      bool IsCommandCompleted(bool Input);

    private:

      const double mDefaultValue;
  };
}
