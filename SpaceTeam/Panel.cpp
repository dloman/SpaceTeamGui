#include "Panel.hpp"
#include <HardwareInterface/Types.hpp>

#include <Tcp/Session.hpp>
#include <fmt/format.h>
#include <bitset>

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
  boost::property_tree::ptree& Tree,
  std::shared_ptr<dl::tcp::Session>& pSession)
: mGame(Tree),
  mpSession(pSession),
  mUpdates(),
  mIsConnected(true),
  moSerial(std::nullopt)
{
  mpSession->GetOnDisconnectSignal().Connect(
    [this]
    {
      mIsConnected = false;
    });

  mpSession->GetOnRxSignal().Connect(
    [this] (const std::string& Bytes)
    {
      if (static_cast<eDeviceID>(Bytes[0]) == eDeviceID::eDigital)
      {
        uint64_t Serial;

        std::memcpy(&Serial, Bytes.data() + 1, 8);

        {
          std::lock_guard Lock(mMutex);

          moSerial = Serial;
        }

        uint64_t Data;

        std::memcpy(&Data, Bytes.data() + 9, 8);

        std::bitset<64> Bits(Data);

        for (unsigned i = 0; i < 40; ++i)
        {
          mUpdates.Add(st::Update{
            .mPiSerial = Serial,
            .mId = i,
            .mValue = static_cast<uint8_t>(Bits[i])});
        }
      }
      else if (static_cast<eDeviceID>(Bytes[0]) == eDeviceID::eAnalog)
      {
        uint64_t Serial;

        std::memcpy(&Serial, Bytes.data() + 1, 8);

        {
          std::lock_guard Lock(mMutex);

          moSerial = Serial;
        }

        std::array<uint8_t, 48> Data;

        std::memcpy(Data.data(), Bytes.data() + 9, 8);

        //fmt::print("Data = ");

        //fmt::print("Data = {},{},{}\n", Data[27], Data[28], Data[29]);

        for (unsigned i = 0; i < 32; ++i)
        {
          mUpdates.Add(st::Update{
            .mPiSerial = Serial,
            .mId = i,
            .mValue = Data[i]});

          //fmt::print("{},", Data[i]);
        }
        //fmt::print("\n");
      }
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Panel::GetIsConnected() const
{
  return mIsConnected;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::optional<uint64_t> Panel::GetSerial() const
{
  std::lock_guard Lock(mMutex);

  return moSerial;
}
