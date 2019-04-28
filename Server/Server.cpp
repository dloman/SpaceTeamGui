#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Panel.hpp>
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Tcp/Server.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fmt/format.h>
#include <mutex>

std::chrono::time_point<std::chrono::system_clock> gGpioToggle(std::chrono::seconds(0));

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t GetRoundSize(size_t NumberOfPanels, size_t RoundNumber)
{
  return 30; //NumberOfPanels * 5 + 2*(RoundNumber / 5);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGameOver(std::vector<std::unique_ptr<st::Panel>>& Panels)
{
  if (Panels.empty())
  {
    return;
  }

  auto& FirstGame = Panels.front()->mGame;

  auto Indecies = FirstGame.GetNextRoundInputs(30);

  for (auto& pPanel : Panels)
  {
    boost::property_tree::ptree Tree;

    Tree.put(
      "reset",
      "Game Over. You crashed and died a horrible painful space death. Good try though!");

    Tree.put("wait", true);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    boost::property_tree::write_json(std::cout, Tree);

    pPanel->mGame.SetCurrentRound(1);

    pPanel->mGame.SetNextRoundInputs(Indecies);

    pPanel->mpSession->Write(Stream.str());
  }

  std::terminate();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendNewRound(std::vector<std::unique_ptr<st::Panel>>& Panels)
{
  if (Panels.empty())
  {
    return;
  }

  auto& FirstGame = Panels.front()->mGame;

  auto Indecies = FirstGame.GetNextRoundInputs(30);

  for (auto& pPanel : Panels)
  {
    boost::property_tree::ptree Tree;

    const auto LastRound = pPanel->mGame.GetCurrentRound();

    pPanel->mGame.SetCurrentRound(LastRound + 1);

    pPanel->mGame.SetNextRoundInputs(Indecies);

    Tree.put(
      "reset",
      fmt::format(
        "Round {} Completed. Good job so far, don't screw it up. You're almost there. Get Ready for Round {}!\n",
        LastRound,
        pPanel->mGame.GetCurrentRound()));

    Tree.put("wait", true);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    boost::property_tree::write_json(std::cout, Tree);

    pPanel->mpSession->Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
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
void SendGpioDirection(st::Game& Game, std::shared_ptr<dl::tcp::Session>& pSession)
{
  for (const auto Serial : Game.GetPiSerials())
  {
    boost::property_tree::ptree Tree;

    Tree.put("gpioDirection", Game.GetHardwareDirection(Serial));

    Tree.put("PiSerial", Serial);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    pSession->Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGpioValue(std::vector<std::unique_ptr<st::Panel>>& Panels)
{
  for (auto& pPanel : Panels)
  {
    for (const auto Serial : pPanel->mGame.GetPiSerials())
    {
      boost::property_tree::ptree Tree;

      Tree.put("gpioValue", pPanel->mGame.GetHardwareValue(Serial));

      Tree.put("PiSerial", Serial);

      std::stringstream Stream;

      boost::property_tree::write_json(Stream, Tree);

      pPanel->mpSession->Write(Stream.str());
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OnError(const std::string& Error)
{
  std::cerr << "Error = " << Error << std::endl;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  std::mutex Mutex;

  st::UpdateVec Updates;

  std::vector<std::unique_ptr<st::Panel>> Panels;

  boost::property_tree::ptree Tree;

  boost::property_tree::read_json("Setup.json", Tree);

  dl::tcp::Server TcpServer(dl::tcp::ServerSettings{
    .mPort = 8181,
    .mNumberOfIoThreads = 2,
    .mNumberOfCallbackThreads = 2,
    .mOnNewSessionCallback =
      [&] (std::shared_ptr<dl::tcp::Session> pSession)
      {
        fmt::print("New Connection!!\n");

        //std::lock_guard Lock(Mutex);

        Panels.emplace_back(std::make_unique<st::Panel>(Updates, Tree, pSession));

        auto& Panel = *(Panels.back());

        Panel.mGame.SetCurrentRound(1);

        Panel.mGame.GetNextRoundInputs(30);

        SendGpioDirection(Panel.mGame, pSession);
      }});

  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    std::lock_guard Lock(Mutex);

    for (auto& pPanel : Panels)
    {
      auto& Game = pPanel->mGame;

      auto& pSession = pPanel->mpSession;

      Game.UpdateCurrentState(Updates);

      const auto Success = Game.GetSuccess();

      if (Success.mIsActiveCompleted)
      {
        SendReset(Game, pSession);

        Game.Success(true);

        fmt::print("success\n");
        fmt::print("score = {}\n", Game.GetCurrentScore());
      }

      if (Success.mInactiveFailCount > 0)
      {
        Game.Success(false);

        fmt::print("fail\n");
        fmt::print("score = {}\n", Game.GetCurrentScore());
      }

      if (std::chrono::system_clock::now() - Game.GetLastResetTime() > std::chrono::seconds(20))
      {
        SendReset(Game, pSession);

        Game.Success(false);
        fmt::print("fail\n");
        fmt::print("score = {}\n", Game.GetCurrentScore());
      }

      const auto CurrentScore = Game.GetCurrentScore();

      if (CurrentScore <= 0)
      {
        SendGameOver(Panels);
      }
      else if (CurrentScore >= 150)
      {
        SendNewRound(Panels);
      }
    }

    Panels.erase(
      std::remove_if(
        Panels.begin(),
        Panels.end(),
        [](const auto& pPanel)
        {
          return !pPanel->GetIsConnected();
        }),
      Panels.end());


    if (std::chrono::system_clock::now()- gGpioToggle > std::chrono::seconds(1))
    {
      for (auto& pPanel : Panels)
      {
        pPanel->mGame.UpdateOutputs();
      }

      gGpioToggle = std::chrono::system_clock::now();

    }
    SendGpioValue(Panels);
  }
}
