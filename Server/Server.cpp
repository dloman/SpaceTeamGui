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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OnError(const std::string& Error)
{
  std::cerr << "Error = " << Error << std::endl;
}

static bool HasSerials(std::vector<std::unique_ptr<st::Panel>>& Panels)
{
  for (const auto& pPanel : Panels)
  {
    if(!pPanel->GetSerial())
    {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
st::Game GetGameStart(
  std::mutex& Mutex,
  std::vector<std::unique_ptr<st::Panel>>& Panels,
  boost::property_tree::ptree& Tree)
{
  fmt::print("Type enter to Begin game\n");

  //std::string unused;
  //std::cin >> unused;

  std::atomic<bool> Temp(true);

  while (Temp)
  {
    std::lock_guard Lock(Mutex);

    if (Panels.size() == 3 && HasSerials(Panels))
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      Temp = false;
    }
  }

  Temp = true;

  while (Temp)
  {
    std::lock_guard Lock(Mutex);

    Temp = !std::all_of(
      Panels.begin(),
      Panels.end(),
      [] (const auto& pPanel) {return pPanel->GetSerial();});
  }

  return st::Game(Tree, Panels);
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

  for(const auto& pPanel : Panels)
  {
    CurrentGamePanels.emplace_back(pPanel.get());
  }

  std::this_thread::sleep_for(5s);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void GetUpdatesFromPanels(
  st::UpdateVec& Updates,
  std::vector<observer_ptr<st::Panel>> CurrentGamePanels)
{
  for (auto& pPanel : CurrentGamePanels)
  {
    pPanel->mUpdates.ForEachAndClear(
      [&Updates] (st::Update& Update)
      {
        Updates.Add(Update);
      });
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

  std::chrono::time_point<std::chrono::system_clock> GpioToggle(std::chrono::seconds(0));

  dl::tcp::Server TcpServer(dl::tcp::ServerSettings{
    .mPort = 8181,
    .mOnNewSessionCallback =
      [&] (std::shared_ptr<dl::tcp::Session> pSession)
      {
        fmt::print("New Connection!!\n");

        SendWaitingForGameToStart(*pSession);

        std::lock_guard Lock(Mutex);

        Panels.emplace_back(std::make_unique<st::Panel>(pSession));
      }});

  auto Game = GetGameStart(Mutex, Panels, Tree);

  StartGame(Mutex, Game, Panels, CurrentGamePanels);

  while(true)
  {
    std::lock_guard Lock(Mutex);

    GetUpdatesFromPanels(Updates, CurrentGamePanels);

    Game.UpdateCurrentState(Updates);

    Updates.Clear();

    const auto CurrentScore = Game.GetCurrentScore();

    if (CurrentScore <= 0)
    {
      SendGameOver(CurrentGamePanels, Game);

      return 1;
    }
    else if (CurrentScore >= 100)
    {
      Game.SendNewRound();
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
