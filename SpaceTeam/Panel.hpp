#pragma once
#include "Game.hpp"

#include <SpaceTeam/Update.hpp>
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
        boost::property_tree::ptree& Tree,
        std::shared_ptr<dl::tcp::Session>& pSession);

      void OnError(const std::string&);

      st::Game mGame;

      std::shared_ptr<dl::tcp::Session> mpSession;

      bool GetIsConnected() const;

      st::UpdateVec mUpdates;

    private:

      std::atomic<bool> mIsConnected;
  };
}
