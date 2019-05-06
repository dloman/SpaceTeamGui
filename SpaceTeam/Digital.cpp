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
std::string Digital::GetNewCommand()
{
  mIsActive = true;

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
  if (mIsActive)
  {
    if (IsInCorrectState())
    {
      mIsActive = false;

      Success.mIsActiveCompleted = true;
    }
    return;
  }

  const auto Correct = IsInCorrectState();

  if (!Correct)
  {
    mDesiredState = mCurrentState;

    Success.mInactiveFailCount++;
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
void Digital::Update(const st::Update& Update)
{
  if (
    Update.mPiSerial != mPiSerial ||
    Update.mId != mId)
  {
    return;
  }

  if (mCurrentState != Update.mValue)
  {
    fmt::print("Id {} changed\n", mId);
  }
  mCurrentState = static_cast<bool>(Update.mValue);
}
