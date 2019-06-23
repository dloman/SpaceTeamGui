#pragma once


#include <boost/property_tree/ptree.hpp>
#include <SpaceTeam/Id.hpp>

namespace st
{
  class Input
  {
    public:

      st::ButtonId GetId() const;

      st::SerialId GetPiSerial() const;

      const std::string& GetLabel() const;

      std::optional<SerialId> GetIsActive() const;

      void ClearActive();

    protected:

      Input(const boost::property_tree::ptree& Tree);

      const st::SerialId mPiSerial;

      const st::ButtonId mId;

      const std::string mLabel;

      std::optional<st::SerialId> mIsActive;
  };
}
