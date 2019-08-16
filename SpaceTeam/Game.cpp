#include "Game.hpp"
#include "Panel.hpp"
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Utility/Visitor.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <bitset>
#include <fmt/ostream.h>
#include <experimental/memory>
#include <random>

using st::Game;

template <typename T>
using observer_ptr = std::experimental::observer_ptr<T>;

using namespace std::chrono_literals;

namespace
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  st::SerialId GetSerial(const std::string& SerialString)
  {
    uint64_t Serial;

    std::stringstream Stream;

    Stream << std::hex << SerialString;

    Stream >> Serial;

    return st::SerialId(Serial);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::unordered_set<st::SerialId> GetSerials(const boost::property_tree::ptree& Tree)
  {
    std::unordered_set<st::SerialId> Serials;

    for (const auto& [Name, SubTree] : Tree)
    {
      Serials.insert(GetSerial(SubTree.get<std::string>("PiSerial")));
    }

    return Serials;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  st::InputVariant GetInput(const boost::property_tree::ptree& Tree)
  {
    const auto Type = Tree.get<std::string>("Type");

    if (Type == std::string("Analog"))
    {
      return st::Analog(Tree);
    }
    else if (Type == std::string("Digital"))
    {
      return st::Digital(Tree);
    }
    else if (Type == std::string("Momentary"))
    {
      return st::Momentary(Tree);
    }

    throw std::logic_error(fmt::format("unreachable {} line {}", __FILE__, __LINE__));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  st::Output GetOutput(const boost::property_tree::ptree& Tree)
  {
    return st::Output{
      .mPiSerial = GetSerial(Tree.get<std::string>("PiSerial")),
      .mId = st::OutputId(Tree.get<unsigned>("Id")),
      .mCurrentState = false,
      .mInput = st::ButtonId(Tree.get<st::ButtonIndex>("Input"),GetSerial(Tree.get<std::string>("PiSerial")))};
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::vector<st::InputVariant> ConstructInputs(boost::property_tree::ptree& Tree)
  {
    std::vector<st::InputVariant> Inputs;

    for (const auto& [Label, SubTree]: Tree)
    {
      if (Label == std::string("Input"))
      {
        Inputs.emplace_back(GetInput(SubTree));
      }
    }

    return Inputs;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::vector<st::Output> ConstructOutputs(boost::property_tree::ptree& Tree)
  {
    std::vector<st::Output> Inputs;

    for (const auto& [Label, SubTree]: Tree)
    {
      if (Label == std::string("Output"))
      {
        Inputs.emplace_back(GetOutput(SubTree));
      }
    }

    return Inputs;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::unordered_map<st::SerialId, std::shared_ptr<dl::tcp::Session>> GetSerialToSessions(
    const std::vector<std::unique_ptr<st::Panel>>& Panels)
  {
    std::unordered_map<st::SerialId, std::shared_ptr<dl::tcp::Session>> SerialToSession;

    for (const auto& pPanel : Panels)
    {
      if (auto oSerial = pPanel->GetSerial())
      {
        SerialToSession[*oSerial] = pPanel->mpSession;
      }
    }

    return SerialToSession;
  }
}

int Game::mCurrentScore = StartingScore;
int Game::mCurrentRound = 0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Game::Game(
  boost::property_tree::ptree& Tree,
  const std::vector<std::unique_ptr<st::Panel>>& Panels)
: mInputs(ConstructInputs(Tree)),
  mCurrentActiveVariants(),
  mOutputs(ConstructOutputs(Tree)),
  mSerialToSession(GetSerialToSessions(Panels)),
  mSerials(GetSerials(Tree)),
  mLastResetTime(),
  mStats()
{
  SendGpioDirections();

  SendGpioValues();

  GetNextRoundInputs();

  for (const auto& Serial : mSerials)
  {
    GetInitialInputDisplay(Serial);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::GetNextRoundInputs()
{
  mCurrentRoundInputs.clear();

  auto ClearActive = [] (auto& InputVariant)
  {
    return std::visit(
      [] (auto& Input) { return Input.ClearActive(); },
      InputVariant);
  };

  for(auto& InputVariant : mInputs)
  {
    ClearActive(InputVariant);
  }

  const size_t RoundSizePerPanel = [this]
  {
    const auto RoundSize = GetRoundSizePerPanel();

    if (RoundSize * mSerialToSession.size() > mInputs.size())
    {
      return mInputs.size() / mSerialToSession.size();
    }

    return RoundSize;
  }();

  std::unordered_set<st::SerialId> ToggleSerials;
  for (auto& InputVariant : mInputs)
  {
    auto GetSerial = [] (auto& InputVariant)
    {
      return std::visit(
        [] (auto& Input) { return Input.GetPiSerial(); },
        InputVariant);
    };
    ToggleSerials.insert(GetSerial(InputVariant));
  }

  for (const auto& Serial : ToggleSerials)
  {
    fmt::print("input Serial {}\n", Serial);
  }
  for (const auto& [SerialNumber, pSession] : mSerialToSession)
  {
    fmt::print("session Serial {}\n", SerialNumber);
    std::vector<std::reference_wrapper<InputVariant>> PanelInputs;

    auto GetSerial = [] (auto& InputVariant)
    {
      return std::visit(
        [] (auto& Input) { return Input.GetPiSerial(); },
        InputVariant);
    };

    for (auto& InputVariant : mInputs)
    {
      if (SerialNumber == GetSerial(InputVariant))
      {
        PanelInputs.push_back(InputVariant);
      }
    }

    static auto Generator = std::mt19937{std::random_device{}()};

    std::sample(
      PanelInputs.begin(),
      PanelInputs.end(),
      std::back_inserter(mCurrentRoundInputs),
      std::min(RoundSizePerPanel, mInputs.size() - mCurrentRoundInputs.size()),
      Generator);

  std::shuffle(mCurrentRoundInputs.begin(), mCurrentRoundInputs.end(), Generator);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Game::GetNextInputDisplay(st::SerialId Serial)
{
  auto IsCurrentlyUsed = [] (auto& InputVariant, auto& CurrentVariant)
  {
    auto GetId = [] (auto& Variant)
    {
      return std::visit([] (auto& Input) { return Input.GetId(); }, Variant);
    };

    auto GetIsActive = [] (auto& Variant)
    {
      return std::visit([] (auto& Input) { return Input.GetIsActive(); }, Variant);
    };

  auto Id = GetId(InputVariant);
  bool Active = GetIsActive(InputVariant);
  auto Current = GetId(CurrentVariant);
  fmt::print("Id = {} GetIsActivve = {}, CurrentId = {}\n", Id, Active, Current);

    return (GetIsActive(InputVariant) || GetId(InputVariant) == GetId(CurrentVariant));
  };

  std::vector<std::reference_wrapper<InputVariant>> RandomCurrentRoundInputs;

  static auto Generator = std::mt19937{std::random_device{}()};

  std::sample(
    mCurrentRoundInputs.begin(),
    mCurrentRoundInputs.end(),
    std::back_inserter(RandomCurrentRoundInputs),
    mCurrentRoundInputs.size(),
    Generator);

  std::shuffle(RandomCurrentRoundInputs.begin(), RandomCurrentRoundInputs.end(), Generator);

  if (!mCurrentActiveVariants.count(Serial))
  {
    throw std::logic_error(fmt::format("unreachable {} line {}", __FILE__, __LINE__));
  }

  auto& CurrentActiveInput = mCurrentActiveVariants[Serial]->get();

  for (auto& rInputVariant : RandomCurrentRoundInputs)
  {
    if (!IsCurrentlyUsed(rInputVariant.get(), CurrentActiveInput))
    {
      mCurrentActiveVariants[Serial] = rInputVariant;

      mLastResetTime[Serial] = std::chrono::system_clock::now();

      return std::visit(
        [Serial] (auto& Input) { return Input.GetNewCommand(Serial);},
        mCurrentActiveVariants[Serial]->get());
    }
  }

  throw std::logic_error(fmt::format("unreachable {} line {}", __FILE__, __LINE__));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::GetInitialInputDisplays()
{
  GetNextRoundInputs();

  for (const auto Serial : mSerials)
  {
    GetInitialInputDisplay(Serial);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::GetInitialInputDisplay(st::SerialId Serial)
{
  if (mCurrentRoundInputs.empty())
  {
    return;
  }

  auto IsActive = [] (auto& Variant)
  {
    return std::visit([] (auto& Input) { return Input.GetIsActive(); }, Variant);
  };

  std::vector<std::reference_wrapper<InputVariant>> RandomCurrentRoundInputs;

  static auto Generator = std::mt19937{std::random_device{}()};

  std::sample(
    mCurrentRoundInputs.begin(),
    mCurrentRoundInputs.end(),
    std::back_inserter(RandomCurrentRoundInputs),
    mCurrentRoundInputs.size(),
    Generator);

  std::shuffle(RandomCurrentRoundInputs.begin(), RandomCurrentRoundInputs.end(), Generator);

  for (auto& rInputVariant : RandomCurrentRoundInputs)
  {
    if (!IsActive(rInputVariant.get()))
    {
      mCurrentActiveVariants[Serial] = rInputVariant;

      mLastResetTime[Serial] = std::chrono::system_clock::now();

      return;
    }
  }

  throw std::logic_error(fmt::format("unreachable {} line {}", __FILE__, __LINE__));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateCurrentState(st::UpdateVec& Updates)
{
  Updates.ForEach(
    [this] (st::Update& Update)
  {
    for (auto& InputVariant : mInputs)
    {
      std::visit(
        [&Update] (auto& Input)
        {
          return Input.Update(Update);
        },
        InputVariant);
    }
  });

  auto Success = GetSuccess();

  for (auto Serial : Success.mIsActiveCompleted)
  {
    fmt::print("Success!\n");
    fmt::print("score = {}\n", GetCurrentScore());

    auto& Session = *(mSerialToSession[Serial]);

    SendReset(Serial, Session);

    UpdateScore(true);
  }

  if (Success.mInactiveFailCount > 0)
  {
    fmt::print("fail\n");
    fmt::print("score = {}\n", GetCurrentScore());

    UpdateScore(false);
  }

  for (const auto& [Serial, pSession] : mSerialToSession)
  {
    if (std::chrono::system_clock::now() - GetLastResetTime(Serial) > 20s)
    {
      fmt::print("fail\n");
      fmt::print("score = {}\n", GetCurrentScore());

      auto ClearActive = [] (auto& InputVariant)
      {
        return std::visit(
          [] (auto& Input) { return Input.ClearActive(); },
          InputVariant);
      };

      ClearActive(mCurrentActiveVariants[Serial]->get());

      SendReset(Serial, *pSession);

      UpdateScore(false);
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
st::Success Game::GetSuccess()
{
  st::Success Success;

  for(auto& InputVariant : mCurrentRoundInputs)
  {
    std::visit(
      [&Success] (auto& Input) { return Input.IsCorrect(Success); },
      InputVariant.get());
  }

  return Success;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::chrono::time_point<std::chrono::system_clock> Game::GetLastResetTime(
  st::SerialId Serial) const
{
  if (mLastResetTime.count(Serial))
  {
    return mLastResetTime.at(Serial);
  }

  using namespace std::chrono_literals;

  return std::chrono::time_point<std::chrono::system_clock>(0s);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateScore(bool Success)
{
  if (Success)
  {
    mCurrentScore += 10;
  }
  else
  {
    mCurrentScore -= 1;
  }

  SendScore();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Game::GetCurrentScore() const
{
  return mCurrentScore;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SetCurrentScore(int Score)
{
  mCurrentScore = Score;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Game::GetCurrentRound() const
{
  return mCurrentRound;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SetCurrentRound(int Round)
{
  mCurrentRound = Round;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
st::HardwareDirection Game::GetHardwareDirection(st::SerialId PiSerial) const
{
  std::bitset<64> Bits(std::numeric_limits<uint64_t>::max());

  for (const auto& Output : mOutputs)
  {
    if (Output.mPiSerial == PiSerial)
    {
      Bits[Output.mId.get()] = false;
    }
  }

  return st::HardwareDirection(Bits.to_ullong());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
st::HardwareValue Game::GetHardwareValue(st::SerialId PiSerial) const
{
  std::bitset<64> Bits(std::numeric_limits<uint64_t>::max());

  for (const auto& Output : mOutputs)
  {
    if (Output.mPiSerial == PiSerial)
    {
      Bits[Output.mId.get()] = Output.mCurrentState;
    }
  }

  return st::HardwareValue(Bits.to_ullong());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateOutputs()
{
  auto GetId = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetId(); }, InputVariant);};

  for (auto& Output : mOutputs)
  {
    auto IsInCurrentRound = (std::find_if(
        mCurrentRoundInputs.begin(),
        mCurrentRoundInputs.end(),
        [Id = Output.mInput, &GetId] (const auto& Input)
        {
          return Id == GetId(Input.get());
        }) == mCurrentRoundInputs.end());

    Output.mCurrentState = IsInCurrentRound;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::vector<st::InputVariant>& Game::GetInputs() const
{
  return mInputs;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::vector<st::Output>& Game::GetOutputs() const
{
  return mOutputs;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Game::GetRoundSizePerPanel()
{
  return 5 + (2 * (mCurrentRound / 3));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::vector<std::reference_wrapper<st::InputVariant>> Game::GetCurrentRoundInputs() const
{
  return mCurrentRoundInputs;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::unordered_set<st::SerialId>& Game::GetPiSerials() const
{
  return mSerials;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SendGpioDirections()
{
  for (const auto& [Serial, pSession] : mSerialToSession)
  {
    boost::property_tree::ptree Tree;

    Tree.put("gpioDirection", GetHardwareDirection(Serial));

    Tree.put("PiSerial", Serial);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    pSession->Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SendGpioValues()
{
  for (const auto& [Serial, pSession] : mSerialToSession)
  {
    boost::property_tree::ptree Tree;

    Tree.put("gpioValue", GetHardwareValue(Serial));

    Tree.put("PiSerial", Serial);

    std::stringstream Stream;

    boost::property_tree::write_json(Stream, Tree);

    pSession->Write(Stream.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SendReset(
  st::SerialId SerialId,
  dl::tcp::Session& Session,
  std::string Value)
{
  boost::property_tree::ptree Tree;

  if (Value.empty())
  {
    Value = GetNextInputDisplay(SerialId);
  }

  Tree.put("reset", Value);

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  boost::property_tree::write_json(std::cout, Tree);

  Session.Write(Stream.str());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SendNewRound()
{
  GetNextRoundInputs();

  const auto CurrentRound = GetCurrentRound() + 1;

  SetCurrentRound(CurrentRound);

  SetCurrentScore(st::StartingScore);

  //Debug
  fmt::print("{}", "NewRound = [");

  for (const auto& InputVariant : GetCurrentRoundInputs())
  {
    fmt::print("{},", std::visit(st::Visitor{
      [] (st::Momentary& Input) { return Input.GetMessage(); },
      [] (auto& Input) { return Input.GetLabel(); }},
      InputVariant.get()));
  }

  fmt::print("]\n\n");
  //Debug

  UpdateOutputs();

  for (auto& [SerialId, pSession] : mSerialToSession)
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

    pSession->Write(Stream.str());
  }

  std::this_thread::sleep_for(15s);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SendScore()
{
  boost::property_tree::ptree Tree;

  Tree.put("score", mCurrentScore);

  std::stringstream Stream;

  boost::property_tree::write_json(Stream, Tree);

  auto Bytes = Stream.str();

  for (const auto& [Serial, pSession] : mSerialToSession)
  {
    pSession->Write(Bytes);
  }
}
