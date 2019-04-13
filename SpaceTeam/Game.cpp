#include "Game.hpp"
#include <SpaceTeam/Success.hpp>
#include <SpaceTeam/Update.hpp>
#include <Utility/Visitor.hpp>
#include <boost/property_tree/ptree.hpp>
#include <random>

using st::Game;

namespace
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  uint64_t GetSerial(const std::string& SerialString)
  {
    uint64_t Serial;

    std::stringstream Stream;

    Stream << std::hex << SerialString;

    Stream >> Serial;

    return Serial;
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
      .mId = Tree.get<unsigned>("Id") - 1,
      .mCurrentState = false};
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::vector<st::InputVariant> GetInputs(boost::property_tree::ptree& Tree)
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
  std::vector<st::Output> GetOutputs(boost::property_tree::ptree& Tree)
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
  std::unordered_set<uint64_t> GetSerials(boost::property_tree::ptree& Tree)
  {
    std::unordered_set<uint64_t> Serials;

    for (const auto& [Label, SubTree]: Tree)
    {
       Serials.insert(GetSerial(SubTree.get<std::string>("PiSerial")));

       (void)Label;
    }

    return Serials;
  }
}

int Game::mCurrentScore = 100;
int Game::mCurrentRound = 100;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Game::Game(boost::property_tree::ptree& Tree)
: mInputs(GetInputs(Tree)),
  moCurrentActiveVariant(std::nullopt),
  mOutputs(GetOutputs(Tree)),
  mPiSerials(GetSerials(Tree)),
  mLastResetTime(std::chrono::milliseconds(0))
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Game::GetRoundInputsSize() const
{
  return 1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::GetNextRoundInputs()
{
  mCurrentRoundInputs.clear();

  while (mCurrentRoundInputs.size() < GetRoundInputsSize())
  {
    auto& InputVariant = mInputs[GetRandomInputIndex(mInputs.size())];

    auto GetId = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetId(); }, InputVariant);};

    if (
      std::find_if(
        mCurrentRoundInputs.begin(),
        mCurrentRoundInputs.end(),
        [Id = GetId(InputVariant), &InputVariant, &GetId] (const auto& Input)
        {
          return Id == GetId(Input.get());
        }) == mCurrentRoundInputs.end())
    {
      mCurrentRoundInputs.emplace_back(InputVariant);
    }
  }

  mCurrentScore = 100;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Game::GetNextInputDisplay()
{
  moCurrentActiveVariant = *std::next(
    mCurrentRoundInputs.begin(),
    GetRandomInputIndex(mCurrentRoundInputs.size()));

  mLastResetTime = std::chrono::system_clock::now();

  return std::visit(
    [] (auto& Input) { return Input.GetNewCommand();},
    moCurrentActiveVariant->get());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateCurrentState(st::UpdateVec& Updates)
{
  Updates.ForEachAndClear(
    [this] (const st::Update& Update)
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
std::chrono::time_point<std::chrono::system_clock> Game::GetLastResetTime() const
{
  return mLastResetTime;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::Success(bool Success)
{
  if (Success)
  {
    mCurrentScore += 10;
  }
  else
  {
    mCurrentScore -= 49;
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

  GetNextRoundInputs();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint64_t Game::GetHardwareDirection(uint64_t PiSerial) const
{
  std::bitset<64> Bits(std::numeric_limits<uint64_t>::max());

  for (const auto& Output : mOutputs)
  {
    if (Output.mPiSerial == PiSerial)
    {
      Bits[Output.mId] = false;
    }
  }

  return Bits.to_ulong();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint64_t Game::GetHardwareValue(uint64_t PiSerial) const
{
  std::bitset<64> Bits(0);

  for (const auto& Output : mOutputs)
  {
    if (Output.mPiSerial == PiSerial)
    {
      Bits[Output.mId] = Output.mCurrentState;
    }
  }

  return Bits.to_ulong();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::unordered_set<uint64_t>& Game::GetPiSerials() const
{
  return mPiSerials;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateOutputs()
{
  for (auto& Output : mOutputs)
  {
    Output.mCurrentState = !Output.mCurrentState;
  }
}
