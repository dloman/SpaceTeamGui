#include "Input.hpp"

using st::Input;

namespace
{
  uint64_t GetSerial(const std::string& SerialString)
  {
    uint64_t Serial;

    std::stringstream Stream;

    Stream << std::hex << SerialString;

    Stream >> Serial;

    return Serial;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Input::Input(const boost::property_tree::ptree& Tree)
: mPiSerial(GetSerial(Tree.get<std::string>("PiSerial"))),
  mId(Tree.get<unsigned>("Id") - 1),
  mLabel(Tree.get<std::string>("Label")),
  mIsActive(false)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned Input::GetId() const
{
  return mId;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
uint64_t Input::GetPiSerial() const
{
  return mPiSerial;
}
