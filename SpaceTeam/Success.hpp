#pragma once

#include <unordered_set>

namespace st
{
  struct Success
  {
    std::unordered_set<st::SerialId> mIsActiveCompleted;

    std::unordered_set<st::SerialId> mInactiveFails;
  };
}

