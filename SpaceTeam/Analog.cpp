#include "Analog.hpp"
#include <Utility/Random.hpp>
#include <fmt/format.h>

namespace
{
  constexpr double MaxVoltage = 5.0;
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  double GetRandomValue()
  {
    return st::random::GetUniform(0.0, MaxVoltage);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  template<typename ContainerType, typename ValueType>
  typename ContainerType::const_iterator FindClosest(
    ContainerType Container,
    ValueType Value)
  {
    return std::min_element(
      Container.begin(), Container.end(), [&Value](auto& x, auto& y)
      {
        return std::abs(x.mStart - Value) < std::abs(y.mStart - Value);
      });
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  std::vector<st::Threshold> GetThresholds(
    const boost::property_tree::ptree& Tree)
  {
    std::vector<st::Threshold> Thresholds;

    for (const auto& [Name, SubTree] : Tree)
    {
      if (Name == std::string("Threshold"))
      {
        Thresholds.emplace_back(st::Threshold{
          SubTree.get<double>("Start"),
          SubTree.get<double>("Stop"),
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
  mDesiredValue(0.0),
  mThresholds(GetThresholds(Tree))
{
}

#include <iostream>
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string Analog::GetNewCommand()
{
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
bool Analog::IsCommandCompleted() const
{
  return (mDesiredValue == GetThreshold(mCurrentState).mStart);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const st::Threshold& Analog::GetThreshold(double Value) const
{
  const auto iThreshold = FindClosest(mThresholds, Value);

  if (iThreshold == mThresholds.end())
  {
    throw std::logic_error("unreachable");
  }

  return *iThreshold;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Analog::SetCurrentState(double State)
{
  mCurrentState = State;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
double Analog::GetNewValue(const Threshold& CurrentThreshold)
{
  const auto& NewThreshold = GetThreshold(GetRandomValue());

  if (std::abs(NewThreshold.mStart - CurrentThreshold.mStart) > 0.01)
  {
    return GetNewValue(CurrentThreshold);
  }

  return NewThreshold.mStart;
}