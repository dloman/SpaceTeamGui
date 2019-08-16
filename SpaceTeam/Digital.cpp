#include "Digital.hpp"
#include <HardwareInterface/Types.hpp>
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

  mDesiredState = mCurrentState < 40 ? 255u : 0u;

  return fmt::format("{} {} to {}",
    GetVerb(),
    mLabel,
    (mDesiredState ? mOnLabel : mOffLabel));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Digital::IsInCorrectState() const
{
  return std::abs(
    static_cast<int>(mCurrentState) - static_cast<int>(mDesiredState)) < 30;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::IsCorrect(st::Success& Success)
{
  const auto Updates = std::move(mUpdates);

  if (Updates.empty())
  {
    return;
  }

  mCurrentState = std::accumulate(Updates.begin(), Updates.end(), 0)/ Updates.size();

  const auto Correct = IsInCorrectState();

  if (mIsActive)
  {
    if (Correct)
    {
      Success.mIsActiveCompleted.insert(*mIsActive);

      mIsActive = std::nullopt;

      mDesiredState = mCurrentState < 40 ? 0u : 255u;
      fmt::print("dddddsucess\n");
    }
    return;
  }

  if (!Correct)
  {
    mDesiredState = mCurrentState < 40 ? 0u : 255u;

    Success.mInactiveFailCount++;

    fmt::print("ddddfail\n");
  }

  return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::SetCurrentState(uint8_t State)
{
  mCurrentState = State;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint8_t Digital::GetCurrentState() const
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

  mUpdates.push_back(Update.mValue);
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
