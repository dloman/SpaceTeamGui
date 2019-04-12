#include "Panel.hpp"
#include "Update.hpp"

#include <Tcp/Session.hpp>
#include <fmt/format.h>

using st::Panel;

//namespace
//{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void OnConnectionError(const std::string& Error)
  {
    std::cerr << "Error: " << Error << std::endl;
  }
//}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Panel::Panel(
  st::UpdateVec& Updates,
  boost::property_tree::ptree& Tree,
  std::shared_ptr<dl::tcp::Session>& pSession)
: mGame(Tree),
  mpSession(pSession),
  mUpdates(Updates),
  mIsConnected(true)
{
  mpSession->GetOnDisconnectSignal().Connect(
    [this]
    {
      std::lock_guard Lock(mMutex);

      mIsConnected = false;
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Panel::OnRxDigital(std::string_view Bytes)
{
  // construct an update and add it to the mUpdates;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Panel::OnRxAnalog(std::string_view Bytes)
{
  // construct an update and add it to the mUpdates;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Panel::GetIsConnected() const
{
  std::lock_guard Lock(mMutex);

  return mIsConnected;
}
