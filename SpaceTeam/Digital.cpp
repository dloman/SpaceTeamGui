#include "Digital.hpp"
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <fmt/format.h>
#include <random>

using st::Digital;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
  std::string GetVerb()
  {
    const static std::vector<std::string> Verbs
    {
      "Set",
      "Switch",
      "Flip",
      "Change",
      "Adjust",
    };

    static std::random_device RandomDevice;
    static std::mt19937 Generator(RandomDevice());
    std::uniform_int_distribution<> Distribution(0, Verbs.size() - 1);;
    return Verbs[Distribution(Generator)];
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Digital::Digital(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDesiredState(false),
  mCurrentState(false),
  mOnLabel(Tree.get<std::string>("On Label")),
  mOffLabel(Tree.get<std::string>("Off Label"))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string Digital::GetNewCommand(st::SerialId Serial)
{
  mIsActive = Serial;

  mDesiredState = !mCurrentState;

  return fmt::format("{} {} to {}",
    GetVerb(),
    mLabel,
    (mDesiredState ? mOnLabel : mOffLabel));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Digital::IsInCorrectState() const
{
  return (mCurrentState == mDesiredState);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::IsCorrect(st::Success& Success)
{
  const auto Correct = IsInCorrectState();

  if (mIsActive)
  {
    if (IsInCorrectState())
    {
      Success.mIsActiveCompleted.insert(*mIsActive);

      mIsActive = std::nullopt;

      mDesiredState = mCurrentState;
    fmt::print("dddddsucess\n");
    }
    return;
  }

  if (!Correct)
  {
    mDesiredState = mCurrentState;

    Success.mInactiveFailCount++;

    fmt::print("ddddfail\n");
  }

  return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::SetCurrentState(bool State)
{
  mCurrentState = State;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Digital::GetCurrentState() const
{
  return mCurrentState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::Update(const st::Update& Update)
{
  if (
    Update.mPiSerial != mPiSerial ||
    Update.mId != mId)
  {
    return;
  }

  mCurrentState = static_cast<bool>(Update.mValue);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Digital::GetOnLabel() const
{
  return mOnLabel;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Digital::GetOffLabel() const
{
  return mOffLabel;
}
