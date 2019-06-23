#include "Input.hpp"

using st::Input;

namespace
{
  st::SerialId GetSerial(const std::string& SerialString)
  {
    uint64_t Serial;

    std::stringstream Stream;

    Stream << std::hex << SerialString;

    Stream >> Serial;

    return st::SerialId(Serial);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Input::Input(const boost::property_tree::ptree& Tree)
: mPiSerial(GetSerial(Tree.get<std::string>("PiSerial"))),
  mId(Tree.get<unsigned>("Id")),
  mLabel(Tree.get<std::string>("Label")),
  mIsActive(std::nullopt)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
st::ButtonId Input::GetId() const
{
  return mId;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
st::SerialId Input::GetPiSerial() const
{
  return mPiSerial;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string& Input::GetLabel() const
{
  return mLabel;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::optional<st::SerialId> Input::GetIsActive() const
{
  return mIsActive;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Input::ClearActive()
{
  mIsActive = std::nullopt;
}
