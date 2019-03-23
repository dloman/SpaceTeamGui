#pragma once

#include <boost/property_tree/ptree.hpp>

namespace st
{
  class Input
  {
    protected:

      Input(const boost::property_tree::ptree& Tree);

      const unsigned mId;

      const std::string mLabel;
  };
}
