#include "Momentary.hpp"

using st::Momentary;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Momentary::Momentary(const boost::property_tree::ptree& Tree)
: Input(Tree),
  mDefaultValue(Tree.get<double>("Default Value"))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string Momentary::GetNewCommand()
{
  return "Farts";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Momentary::IsCommandCompleted(bool Input)
{
  return Input != mDefaultValue;
}

