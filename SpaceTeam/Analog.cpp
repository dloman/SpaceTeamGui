#include "Analog.hpp"
#include <HardwareInterface/Types.hpp>
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Utility/Random.hpp>
#include <fmt/format.h>

namespace
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  std::vector<st::Threshold> ConstructThresholds(
    const boost::property_tree::ptree& Tree)
  {
    std::vector<st::Threshold> Thresholds;

    for (const auto& [Name, SubTree] : Tree)
    {
      if (Name == std::string("Threshold"))
      {
        auto Start = static_cast<uint8_t>(SubTree.get<int>("Start"));

        auto Stop = static_cast<uint8_t>(SubTree.get<int>("Stop"));

        Thresholds.emplace_back(st::Threshold{
          std::min(Start, Stop),
          std::max(Start, Stop),
          SubTree.get<std::string>("Label")});
      }
    }

    return Thresholds;
  }

}

using st::Analog;

const std::vector<std::string> Analog::mSetWords = {"Set", "Adjust", "Change"};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Analog::Analog(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDesiredValue(0u),
  mUpdateCount(0u),
  mUpdateSum(0u),
  mThresholds(ConstructThresholds(Tree))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string Analog::GetNewCommand(st::SerialId Serial)
{
  mIsActive = Serial;

  mDesiredValue = GetNewValue(GetThreshold(mCurrentState));

  return
    fmt::format(
      "{:s} {:s} to {:s}",
      mSetWords[st::random::GetUniform<size_t>(0, mSetWords.size()-1)],
      mLabel,
      GetThreshold(mDesiredValue).mLabel);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Analog::GetIsInCorrectState() const
{
 return (mDesiredValue == GetThreshold(mCurrentState).mStart);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Analog::IsCorrect(st::Success& Success)
{
  if (!mUpdateCount)
  {
    return;
  }

  mCurrentState = mUpdateSum / mUpdateCount;

  mUpdateCount = mUpdateSum = 0;

  const auto IsInCorrectState = GetIsInCorrectState();

  if (mIsActive)
  {
    if (IsInCorrectState)
    {
      Success.mIsActiveCompleted.insert(*mIsActive);

      mIsActive = std::nullopt;
    }

    return;
  }

  if (!IsInCorrectState)
  {
    mDesiredValue = GetThreshold(mCurrentState).mStart;

    Success.mInactiveFailCount++;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const st::Threshold& Analog::GetThreshold(uint8_t Value) const
{
  auto iThreshold = std::find_if(
    mThresholds.begin(),
    mThresholds.end(),
    [&Value](const auto& x)
    {
      return (Value >= x.mStart && Value <= x.mStop);
    });

  if (iThreshold == mThresholds.end())
  {
    for (auto& y : mThresholds)
    {
      fmt::print("starting thresholds {} {} {}\n", y.mLabel, y.mStart, y.mStop);
    }

    fmt::print("value = {} \n", Value);

    throw std::logic_error("unreachable");
  }

  return *iThreshold;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Analog::SetCurrentState(uint8_t State)
{
  mCurrentState = State;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint8_t Analog::GetNewValue(const Threshold& CurrentThreshold)
{
  std::vector<Threshold> InActiveThresholds;

  std::copy_if(
    mThresholds.begin(),
    mThresholds.end(),
    std::back_inserter(InActiveThresholds),
    [Start = CurrentThreshold.mStart] (const auto& Threshold)
    {
      return Start != Threshold.mStart;
    });

  const auto& NewThreshold = InActiveThresholds.at(
    st::random::GetUniform(
      static_cast<size_t>(0),
      InActiveThresholds.size() - 1));

  return NewThreshold.mStart;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Analog::Update(const st::Update& Update)
{
  if (
    Update.mPiSerial != mPiSerial ||
    Update.mId != mId ||
    Update.mUpdateType != eDeviceID::eAnalog)
  {
    return;
  }

  mUpdateSum += Update.mValue;

  mUpdateCount++;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint8_t Analog::GetCurrentState() const
{
  return mCurrentState;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::vector<st::Threshold>& Analog::GetThresholds() const
{
  return mThresholds;
}

