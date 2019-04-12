#include "Momentary.hpp"
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>

using st::Momentary;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Momentary::Momentary(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDefaultValue(Tree.get<double>("Default Value")),
  mMessage(Tree.get<std::string>("Message")),
  mCurrentState(mDefaultValue),
  mLastToggle(std::chrono::milliseconds(0))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Momentary::GetNewCommand()
{
  mIsActive = true;

  using namespace std::chrono;

  mLastToggle = time_point<system_clock>(milliseconds(0));

  return mMessage;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Momentary::IsPressed()
{
  return mCurrentState != mDefaultValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Momentary::IsCorrect(st::Success& Success)
{
  using namespace std::chrono;

  if ((system_clock::now() - mLastToggle) < milliseconds(60))
  {
    return;
  }

  if (mIsActive)
  {
    if (IsPressed())
    {
      mIsActive = false;

      Success.mIsActiveCompleted = true;
    }

    return;
  }

  if (IsPressed())
  {
    mLastToggle = system_clock::now();

    Success.mInactiveFailCount++;
  }

  return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Momentary::SetCurrentState(bool State)
{
  mCurrentState = State;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Momentary::Update(const st::Update& Update)
{
  if (
    Update.mPiSerial != mPiSerial ||
    Update.mId != mId)
  {
    return;
  }

  //Set State
}
