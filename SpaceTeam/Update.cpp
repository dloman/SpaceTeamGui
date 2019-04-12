#include "Update.hpp"

using st::UpdateVec;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void UpdateVec::Add(const st::Update& Update)
{
  std::lock_guard Lock(mMutex);

  mUpdates.emplace_back(Update);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void UpdateVec::Clear()
{
  std::lock_guard Lock(mMutex);

  mUpdates.clear();
}
