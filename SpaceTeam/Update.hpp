#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

namespace st
{
  struct Update
  {
    const uint64_t mPiSerial;

    const unsigned mId;

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

        for (const auto& Update : mUpdates)
        {
          Callback(Update);
        }
      }


      template <typename CallbackType>
      void ForEachAndClear(CallbackType&& Callback)
      {
        std::lock_guard Lock(mMutex);

        for (const auto& Update : mUpdates)
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
