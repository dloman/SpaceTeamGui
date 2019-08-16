#include "Momentary.hpp"
#include <HardwareInterface/Types.hpp>
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>

using st::Momentary;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Momentary::Momentary(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDefaultValue(Tree.get<uint8_t>("Default Value")),
  mMessage(Tree.get<std::string>("Message")),
  mCurrentState(mDefaultValue),
  mLastToggle(std::chrono::milliseconds(0))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Momentary::GetNewCommand(st::SerialId Serial)
{
  mIsActive = Serial;

  using namespace std::chrono;

  mLastToggle = time_point<system_clock>(milliseconds(0));

  return mMessage;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static bool IsClose(uint8_t Rhs, uint8_t Lhs)
{
  return std::abs(static_cast<int>(Rhs) - static_cast<int>(Lhs)) > 210;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Momentary::WasPressed()
{
  const auto Updates = std::move(mUpdates);

  for (const auto State : Updates)
  {
    fmt::print("{:2x}, {:2x}\n", State, mDefaultValue);
    if (IsClose(State, mDefaultValue))
    {
      return true;
    }
  }
  return false;
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
    if (WasPressed())
    {
      mLastToggle = system_clock::now();

      Success.mIsActiveCompleted.insert(*mIsActive);

      mIsActive = std::nullopt;
    }

    mUpdates.clear();

    return;
  }

  if (WasPressed())
  {
    mLastToggle = system_clock::now();

    fmt::print("momentary fail\n");
    Success.mInactiveFailCount++;
  }

  mUpdates.clear();

  return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Momentary::SetCurrentState(uint8_t State)
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

  mUpdates.push_back(Update.mValue);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Momentary::GetMessage() const
{
  return mMessage;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Momentary::GetDefaultValue() const
{
  return mDefaultValue;
}
