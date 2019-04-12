#pragma once
#include "Game.hpp"

#include <Tcp/Client.hpp>
#include <boost/property_tree/ptree.hpp>

namespace dl::tcp
{
  class Session;
}

namespace st
{
  class UpdateVec;

  class Panel
  {
    public:

      Panel(
        st::UpdateVec& Updates,
        boost::property_tree::ptree& Tree,
        std::shared_ptr<dl::tcp::Session>& pSession);

      void OnRxDigital(std::string_view Bytes);

      void OnRxAnalog(std::string_view Bytes);

      void OnError(const std::string&);

      st::Game mGame;

      std::shared_ptr<dl::tcp::Session> mpSession;

      bool GetIsConnected() const;

    private:

      st::UpdateVec& mUpdates;

      bool mIsConnected;

      mutable std::mutex mMutex;
  };
}
