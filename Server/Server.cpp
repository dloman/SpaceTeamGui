#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Panel.hpp>
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Tcp/Server.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fmt/format.h>
#include <csignal>
#include <mutex>
#include <experimental/memory>

std::chrono::time_point<std::chrono::system_clock> gGpioToggle(std::chrono::seconds(0));

template <typename T>
using observer_ptr = std::experimental::observer_ptr<T>;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGameOver(std::vector<observer_ptr<st::Panel>>& Panels)
{
  if (Panels.empty())
  {
    return;
  }

  auto& FirstGame = Panels.front()->mGame;

  auto Indecies = FirstGame.GetNextRoundInputs();

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
void SendWaitingForGameToStart(observer_ptr<st::Panel>& pPanel)
{
  boost::property_tree::ptree Tree;

  Tree.put(
    "reset",
    "Waiting for New Game to start");

  Tree.put("wait", true);

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  boost::property_tree::write_json(std::cout, Tree);

  pPanel->mGame.SetCurrentRound(1);

  pPanel->mpSession->Write(Stream.str());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendNewRound(std::vector<observer_ptr<st::Panel>>& Panels)
{
  if (Panels.empty())
  {
    return;
  }

  auto& FirstGame = Panels.front()->mGame;

  auto Indecies = FirstGame.GetNextRoundInputs();

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
void SendGpioValue(std::vector<observer_ptr<st::Panel>>& Panels)
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
void GetGameStart(
  std::mutex& Mutex,
  std::vector<std::unique_ptr<st::Panel>>& Panels)
{
  fmt::print("Type enter to Begin game\n");

  //std::string unused;
  //std::cin >> unused;

  std::atomic<bool> Temp(true);

  while (Temp)
  {
    {
      std::lock_guard Lock(Mutex);

      if (Panels.size())
      {
        Temp = false;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void StartGame(
  std::mutex& Mutex,
  std::vector<std::unique_ptr<st::Panel>>& Panels,
  std::vector<observer_ptr<st::Panel>>& CurrentGamePanels)
{
  std::lock_guard Lock(Mutex);

  if (Panels.empty())
  {
    throw std::logic_error("no panels connected");
  }

  auto Indecies = Panels.front()->mGame.GetNextRoundInputs();

  for(const auto& pPanel : Panels)
  {
    CurrentGamePanels.emplace_back(pPanel.get());

    pPanel->mGame.SetCurrentRound(1);

    pPanel->mGame.SetNextRoundInputs(Indecies);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  std::mutex Mutex;

  st::UpdateVec Updates;

  std::vector<std::unique_ptr<st::Panel>> Panels;

  std::vector<observer_ptr<st::Panel>> CurrentGamePanels;

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

        std::lock_guard Lock(Mutex);

        Panels.emplace_back(std::make_unique<st::Panel>(Tree, pSession));

        auto& Panel = *(Panels.back());

        Panel.mGame.SetCurrentRound(0);

        SendGpioDirection(Panel.mGame, pSession);
      }});

  GetGameStart(Mutex, Panels);

  StartGame(Mutex, Panels, CurrentGamePanels);

  while(true)
  {
    std::lock_guard Lock(Mutex);

    for (auto& pPanel : CurrentGamePanels)
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
        SendGameOver(CurrentGamePanels);
      }
      else if (CurrentScore >= 150)
      {
        SendNewRound(CurrentGamePanels);
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

    CurrentGamePanels.erase(
      std::remove_if(
        CurrentGamePanels.begin(),
        CurrentGamePanels.end(),
        [](const auto& pPanel)
        {
          return !pPanel->GetIsConnected();
        }),
      CurrentGamePanels.end());

    if (std::chrono::system_clock::now()- gGpioToggle > std::chrono::seconds(1))
    {
      for (auto& pPanel : CurrentGamePanels)
      {
        pPanel->mGame.UpdateOutputs();
      }

      gGpioToggle = std::chrono::system_clock::now();
    }

    SendGpioValue(CurrentGamePanels);

    Updates.Clear();

    for (auto& pPanel : CurrentGamePanels)
    {
      pPanel->mUpdates.ForEachAndClear(
        [&Updates] (st::Update& Update)
        {
          Updates.Add(Update);
        });
    }
  }


}
