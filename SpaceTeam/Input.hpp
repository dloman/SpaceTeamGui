#pragma once

#include <boost/property_tree/ptree.hpp>

namespace st
{
  class Input
  {
    public:

      unsigned GetId() const;

      uint64_t GetPiSerial() const;

      const std::string& GetLabel() const;

      bool GetIsActive() const;

    protected:

      Input(const boost::property_tree::ptree& Tree);

      const uint64_t mPiSerial;

      const unsigned mId;

      const std::string mLabel;

      bool mIsActive;
  };
}
