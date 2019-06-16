#pragma once

#include <SpaceTeam/Id.hpp>

namespace st
{
  struct Output
  {
    const st::SerialId mPiSerial;

    const st::OutputId mId;

    bool mCurrentState;

    const st::ButtonId mInput;
  };
}
