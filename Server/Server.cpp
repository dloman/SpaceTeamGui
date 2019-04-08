#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Success.hpp>
#include <Tcp/Server.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fmt/format.h>
#include <mutex>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template <typename T>
void SendGameOver(T& Games)
{
  for (auto& [Game, pSession] : Games)
  {
    boost::property_tree::ptree Tree;

    Tree.put(
      "reset",
      "Game Over. You crashed and died a horrible painful space death. Good try though!");

    Tree.put("wait", true);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    boost::property_tree::write_json(std::cout, Tree);

    Game.SetCurrentRound(1);

    pSession->Write(Stream.str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(30));

  std::terminate();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template <typename T>
void SendNewRound(T& Games)
{
  for (auto& [Game, pSession] : Games)
  {
    boost::property_tree::ptree Tree;

    const auto LastRound = Game.GetCurrentRound();

    Game.SetCurrentRound(LastRound + 1);

    Tree.put(
      "reset",
      fmt::format(
        "Round {} Completed. Good job so far, don't screw it up. You're almost there. Get Ready for Round {}!\n",
        LastRound,
        Game.GetCurrentRound()));

    Tree.put("wait", true);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    boost::property_tree::write_json(std::cout, Tree);

    pSession->Write(Stream.str());
  }
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

        auto& [Game, pTemp] = Games.back();

        Game.SetCurrentRound(1);
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

        Game.Success(true);
      }

      if (Success.mInactiveFailCount > 0)
      {
        Game.Success(false);
      }

      if (std::chrono::system_clock::now() - Game.GetLastResetTime() > std::chrono::seconds(20))
      {
        SendReset(Game, pSession);

        Game.Success(false);
      }

      const auto CurrentScore = Game.GetCurrentScore();

      if (CurrentScore <= 0)
      {
        SendGameOver(Games);
      }
      else if (CurrentScore >= 150)
      {
        SendNewRound(Games);
      }
    }
  }
}
