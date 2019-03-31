#include "Input.hpp"

using st::Input;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Input::Input(const boost::property_tree::ptree& Tree)
: mId(Tree.get<unsigned>("Id")),
  mLabel(Tree.get<std::string>("Label"))
{
}