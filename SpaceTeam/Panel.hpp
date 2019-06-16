#pragma once
#include "Game.hpp"

#include <SpaceTeam/Id.hpp>
#include <SpaceTeam/Update.hpp>
#include <Tcp/Client.hpp>

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

      Panel(std::shared_ptr<dl::tcp::Session>& pSession);

      void OnError(const std::string&);

      bool GetIsConnected() const;

      std::shared_ptr<dl::tcp::Session> mpSession;

      st::UpdateVec mUpdates;

      std::optional<st::SerialId> GetSerial() const;

      const st::PanelId mId;

    private:

      static size_t mCount;

      std::atomic<bool> mIsConnected;

      std::optional<st::SerialId> moSerial;

      mutable std::mutex mMutex;
  };
}
