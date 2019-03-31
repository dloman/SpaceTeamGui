#include "Digital.hpp"
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
  mDesiredState = !mCurrentState;

  return fmt::format("{} {} to {}",
    GetVerb(),
    mLabel,
    (mDesiredState ? mOnLabel : mOffLabel));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Digital::IsCommandCompleted() const
{
  return mCurrentState == mDesiredState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Digital::SetCurrentState(bool State)
{
  mCurrentState = State;
}
