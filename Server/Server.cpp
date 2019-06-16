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

template <typename T>
using observer_ptr = std::experimental::observer_ptr<T>;

using namespace std::chrono_literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGameOver(std::vector<observer_ptr<st::Panel>>& Panels, st::Game& Game)
{
  if (Panels.empty())
  {
    return;
  }

  Game.SetCurrentRound(0);

  Game.SetCurrentScore(st::StartingScore);

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

    pPanel->mpSession->Write(Stream.str());
  }

  std::terminate();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendWaitingForGameToStart(dl::tcp::Session& Session)
{
  boost::property_tree::ptree Tree;

  Tree.put(
    "reset",
    "Waiting for New Game to start");

  Tree.put("wait", true);

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  boost::property_tree::write_json(std::cout, Tree);

  Session.Write(Stream.str());
}

#include <Utility/Visitor.hpp>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendNewRound(std::vector<observer_ptr<st::Panel>>& Panels, st::Game& Game)
{
  if (Panels.empty())
  {
    return;
  }

  std::vector<st::SerialId> ActiveSerialNumbers;

  for (const auto& pPanel : Panels)
  {
    if (auto oSerial = pPanel->GetSerial())
    {
      ActiveSerialNumbers.emplace_back(*oSerial);
    }
    else
    {
      throw std::logic_error("unknown serial for panel");
    }
  }

  Game.GetNextRoundInputs(ActiveSerialNumbers);

  const auto CurrentRound = Game.GetCurrentRound() + 1;

  Game.SetCurrentRound(CurrentRound);

  Game.SetCurrentScore(st::StartingScore);

  //Debug
  fmt::print("{}", "NewRound = [");

  for (const auto& InputVariant : Game.GetCurrentRoundInputs())
  {
    fmt::print("{},", std::visit(st::Visitor{
      [] (st::Momentary& Input) { return Input.GetMessage(); },
      [] (auto& Input) { return Input.GetLabel(); }},
      InputVariant.get()));
  }

  fmt::print("]\n\n");
  //Debug

  for (auto& pPanel : Panels)
  {
    boost::property_tree::ptree Tree;

    Tree.put(
      "reset",
      fmt::format(
        "Round {} Completed. Good job so far, don't screw it up. You're almost there. Get Ready for Round {}!\n",
        CurrentRound - 1,
        CurrentRound));

    Tree.put("wait", true);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    pPanel->mpSession->Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendReset(st::Game& Game, st::Panel& Panel)
{
  boost::property_tree::ptree Tree;

  Tree.put("reset", Game.GetNextInputDisplay(*(Panel.GetSerial())));

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  boost::property_tree::write_json(std::cout, Tree);

  Panel.mpSession->Write(Stream.str());

  fmt::print("{} \n","!!!!!!");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGpioDirection(st::Game& Game, dl::tcp::Session& Session)
{
  for (const auto Serial : Game.GetPiSerials())
  {
    boost::property_tree::ptree Tree;

    Tree.put("gpioDirection", Game.GetHardwareDirection(Serial));

    Tree.put("PiSerial", Serial);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    Session.Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendGpioValue(std::vector<observer_ptr<st::Panel>>& Panels, st::Game& Game)
{
  for (auto& pPanel : Panels)
  {
    boost::property_tree::ptree Tree;

    auto Serial = *pPanel->GetSerial();

    Tree.put("gpioValue", Game.GetHardwareValue(Serial));

    Tree.put("PiSerial", Serial);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    pPanel->mpSession->Write(Stream.str());
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
    std::lock_guard Lock(Mutex);

    if (Panels.size() == 2)
    {
      Temp = false;
    }
  }

  Temp = true;
  fmt::print("wtf is going on size == {}\n", Panels.size());

  while (Temp)
  {
    std::lock_guard Lock(Mutex);

    Temp = !std::all_of(
      Panels.begin(),
      Panels.end(),
      [] (const auto& pPanel) {return pPanel->GetSerial();});
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void StartGame(
  std::mutex& Mutex,
  st::Game& Game,
  std::vector<std::unique_ptr<st::Panel>>& Panels,
  std::vector<observer_ptr<st::Panel>>& CurrentGamePanels)
{
  std::lock_guard Lock(Mutex);

  if (Panels.empty())
  {
    throw std::logic_error("no panels connected");
  }

  Game.SetCurrentRound(0);

  Game.SetCurrentScore(st::StartingScore);

  for(const auto& pPanel : Panels)
  {
    CurrentGamePanels.emplace_back(pPanel.get());
  }

  SendNewRound(CurrentGamePanels, Game);

  SendGpioValue(CurrentGamePanels, Game);

  std::this_thread::sleep_for(5s);

  for(const auto& pPanel : Panels)
  {
    SendReset(Game, *pPanel);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void UpdatePanelWithSuccess(st::Panel& Panel, st::Game& Game, const st::Success& Success)
{
  auto Serial = *Panel.GetSerial();

  if (Success.mIsActiveCompleted.contains(Serial))
  {
    fmt::print("success\n");
    fmt::print("score = {}\n", Game.GetCurrentScore());

    SendReset(Game, Panel);

    Game.Success(true, Serial);
  }

  if (Success.mInactiveFailCount > 0)
  {
    fmt::print("fail\n");
    fmt::print("score = {}\n", Game.GetCurrentScore());

    Game.Success(false, Serial);
  }

  if (std::chrono::system_clock::now() - Game.GetLastResetTime(Serial) > 20s)
  {
    fmt::print("fail\n");
    fmt::print("score = {}\n", Game.GetCurrentScore());

    SendReset(Game, Panel);

    Game.Success(false, Serial);
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

  st::Game Game(Tree);

  std::chrono::time_point<std::chrono::system_clock> GpioToggle(std::chrono::seconds(0));

  dl::tcp::Server TcpServer(dl::tcp::ServerSettings{
    .mPort = 8181,
    .mOnNewSessionCallback =
      [&] (std::shared_ptr<dl::tcp::Session> pSession)
      {
        fmt::print("New Connection!!\n");

        SendWaitingForGameToStart(*pSession);

        SendGpioDirection(Game, *pSession);

        std::lock_guard Lock(Mutex);

        Panels.emplace_back(std::make_unique<st::Panel>(pSession));
      }});

  GetGameStart(Mutex, Panels);

  StartGame(Mutex, Game, Panels, CurrentGamePanels);

  while(true)
  {
    std::lock_guard Lock(Mutex);

    Game.UpdateCurrentState(Updates);

    const auto Success = Game.GetSuccess();

    for (auto& pPanel : CurrentGamePanels)
    {
      UpdatePanelWithSuccess(*pPanel, Game, Success);
    }

    const auto CurrentScore = Game.GetCurrentScore();

    if (CurrentScore <= 0)
    {
      SendGameOver(CurrentGamePanels, Game);

      return 1;
    }
    else if (CurrentScore >= 150)
    {
      SendNewRound(CurrentGamePanels, Game);
    }

    //Remove disconnected Panels
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

    //Update Game outputs
    if (std::chrono::system_clock::now()- GpioToggle > 1s)
    {
      Game.UpdateOutputs();

      GpioToggle = std::chrono::system_clock::now();

      SendGpioValue(CurrentGamePanels, Game);
    }

    //Get updates from each panel
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
