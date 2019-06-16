#pragma once

#include <cstdint>
#include <mutex>
#include <vector>
#include <SpaceTeam/Id.hpp>

namespace st
{
  struct Update
  {
    const st::SerialId mPiSerial;

    const st::ButtonId mId;

    const uint8_t mValue;
  };

  class UpdateVec
  {
    public:

      void Add(const st::Update&);

      void Clear();

      template <typename CallbackType>
      void ForEach(CallbackType&& Callback)
      {
        std::lock_guard Lock(mMutex);

        for (auto& Update : mUpdates)
        {
          Callback(Update);
        }
      }


      template <typename CallbackType>
      void ForEachAndClear(CallbackType&& Callback)
      {
        std::lock_guard Lock(mMutex);

        for (auto& Update : mUpdates)
        {
          Callback(Update);
        }

        mUpdates.clear();
      }

    private:

      std::vector<st::Update> mUpdates;

      std::mutex mMutex;
  };
}
