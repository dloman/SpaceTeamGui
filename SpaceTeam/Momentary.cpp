#include "Momentary.hpp"

using st::Momentary;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Momentary::Momentary(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDefaultValue(Tree.get<double>("Default Value")),
  mMessage(Tree.get<std::string>("Message")),
  mCurrentState(mDefaultValue)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Momentary::GetNewCommand()
{
  return mMessage;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Momentary::IsCommandCompleted() const
{
  return mCurrentState != mDefaultValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Momentary::SetCurrentState(bool State)
{
  mCurrentState = State;
}
