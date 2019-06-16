#include "Game.hpp"
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Utility/Visitor.hpp>
#include <boost/property_tree/ptree.hpp>
#include <bitset>
#include <random>

using st::Game;

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
  size_t GetRandomInputIndex(size_t Size)
  {
    static std::random_device RandomDevice;

    static std::mt19937 Generator(RandomDevice());

    std::uniform_int_distribution<> Distribution(0, Size - 1);

    return Distribution(Generator);
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

    throw std::logic_error("unreachable");
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  st::Output GetOutput(const boost::property_tree::ptree& Tree)
  {
    return st::Output{
      .mPiSerial = GetSerial(Tree.get<std::string>("PiSerial")),
      .mId = st::OutputId(Tree.get<unsigned>("Id")),
      .mCurrentState = false,
      .mInput = st::ButtonId(Tree.get<unsigned>("Input"))};
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
  std::unordered_set<st::SerialId> GetSerials(boost::property_tree::ptree& Tree)
  {
    std::unordered_set<st::SerialId> Serials;

    for (const auto& [Label, SubTree]: Tree)
    {
       Serials.insert(GetSerial(SubTree.get<std::string>("PiSerial")));

       (void)Label;
    }

    return Serials;
  }
}

int Game::mCurrentScore = 100;
int Game::mCurrentRound = 0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Game::Game(boost::property_tree::ptree& Tree)
: mInputs(ConstructInputs(Tree)),
  mCurrentActiveVariants(),
  mOutputs(ConstructOutputs(Tree)),
  mPiSerials(GetSerials(Tree)),
  mLastResetTime(),
  mStats()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::GetNextRoundInputs(
  const std::vector<st::SerialId>& ActivePanelSerialNumbers)
{
  mCurrentRoundInputs.clear();

  const size_t RoundSizePerPanel = [this, &ActivePanelSerialNumbers]
  {
    const auto RoundSize = GetRoundSizePerPanel();

    if (RoundSize * ActivePanelSerialNumbers.size() > mInputs.size())
    {
      return mInputs.size() / ActivePanelSerialNumbers.size();
    }

    return RoundSize;
  }();

  for (const auto SerialNumber : ActivePanelSerialNumbers)
  {
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
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Game::GetNextInputDisplay(st::SerialId Serial)
{
  auto GetId = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetId(); }, InputVariant);};

  auto GetIsActive = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetIsActive(); }, InputVariant);};

  auto GetNext = [this] { return *std::next(
    mCurrentRoundInputs.begin(),
    GetRandomInputIndex(mCurrentRoundInputs.size()));};

  auto Next = GetNext();

  fmt::print("next selected for Serial {:x} Id = {}\n",
    Serial.get(),
    GetId(Next.get()));

  if (auto& ActiveVariant = mCurrentActiveVariants[Serial])
  {
    fmt::print("prev id = {}\n", GetId(mCurrentActiveVariants[Serial]->get()));

    // prevent input from being selected if its the panels previous input
    // or selected by antother panel
    if (
      GetId(ActiveVariant->get()) == GetId(Next.get()) ||
      GetIsActive(Next.get()))
    {
      return Game::GetNextInputDisplay(Serial);
    }
  }

  mCurrentActiveVariants[Serial] = Next;

  mLastResetTime[Serial] = std::chrono::system_clock::now();

  return std::visit(
    [Serial] (auto& Input) { return Input.GetNewCommand(Serial);},
    mCurrentActiveVariants[Serial]->get());
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
void Game::Success(bool Success, st::SerialId)
{
  if (Success)
  {
    mCurrentScore += 10;
  }
  else
  {
    mCurrentScore -= 1;
  }
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
const std::unordered_set<st::SerialId>& Game::GetPiSerials() const
{
  return mPiSerials;
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

