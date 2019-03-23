#include "Digital.hpp"
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
      "set",
      "switch",
      "flip",
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
  mOnLabel(Tree.get<std::string>("On Label")),
  mOffLabel(Tree.get<std::string>("Off Label"))
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string Digital::GetNewCommand(double CurrentState)
{
  mDesiredState = CurrentState;

  return GetVerb() + " to " + (mDesiredState ? mOnLabel : mOffLabel);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Digital::IsCommandCompleted(bool State)
{
  return State == mDesiredState;
}
