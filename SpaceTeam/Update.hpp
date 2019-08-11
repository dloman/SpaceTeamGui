#pragma once

#include <cstdint>
#include <mutex>
#include <vector>
#include <SpaceTeam/Id.hpp>

enum class eDeviceID : uint8_t;

namespace st
{
  struct Update
  {
    const st::SerialId mPiSerial;

    const st::ButtonId mId;

    const uint8_t mValue;

    const eDeviceID mUpdateType;
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

      size_t GetSize() const
      {
        std::lock_guard Lock(mMutex);

        return mUpdates.size();
      }

    private:

      std::vector<st::Update> mUpdates;

      mutable std::mutex mMutex;
  };
}
