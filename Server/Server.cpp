#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Success.hpp>
#include <Tcp/Server.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mutex>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGameOver(st::Game& Game, std::shared_ptr<dl::tcp::Session>& pSession)
{
}

//------------------------------------------------------------------------------
void SendReset(st::Game& Game, std::shared_ptr<dl::tcp::Session>& pSession)
{
  boost::property_tree::ptree Tree;

  Tree.put("reset", Game.GetNextInputDisplay());

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  boost::property_tree::write_json(std::cout, Tree);

  pSession->Write(Stream.str());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void UpdateGameState(std::string_view Bytes)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  int CurrentScore = 100;

  std::mutex Mutex;

  std::vector<std::pair<st::Game, std::shared_ptr<dl::tcp::Session>>> Games;

  boost::property_tree::ptree Tree;

  boost::property_tree::read_json("Setup.json", Tree);

  dl::tcp::Server TcpServer(dl::tcp::ServerSettings{
    .mPort = 8181,
    .mNumberOfIoThreads = 2,
    .mNumberOfCallbackThreads = 2,
    .mOnNewSessionCallback =
      [&] (std::shared_ptr<dl::tcp::Session> pSession)
      {
        std::lock_guard Lock(Mutex);

        Games.emplace_back(std::make_pair(st::Game{Tree}, pSession));

        pSession->GetOnRxSignal().Connect(
          [] (const auto& Bytes)
          {
            UpdateGameState(Bytes);
          });
      }});

  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    std::lock_guard Lock(Mutex);

    for (auto& [Game, pSession] : Games)
    {
      const auto Success = Game.GetSuccess();

      if (Success.mIsActiveCompleted)
      {
        SendReset(Game, pSession);

        CurrentScore += 10;
      }

      {
        CurrentScore -= 4;
      }

      if (std::chrono::system_clock::now() - Game.GetLastResetTime() > std::chrono::seconds(20))
      {
        SendReset(Game, pSession);
      }

      if (CurrentScore <= 0)
      {
        SendGameOver(Game, pSession);
      }
    }
  }
}
