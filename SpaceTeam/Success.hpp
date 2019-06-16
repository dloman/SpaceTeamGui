#pragma once

#include <unordered_set>

namespace st
{
  struct Success
  {
    std::unordered_set<st::SerialId> mIsActiveCompleted;

    unsigned mInactiveFailCount = 0;
  };
}

