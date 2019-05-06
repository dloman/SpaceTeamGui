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
      .mId = Tree.get<unsigned>("Id"),
      .mCurrentState = false,
      .mInput = Tree.get<unsigned>("Input")};
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  size_t GetRoundSize()
  {
    return 2;
  }
}

int Game::mCurrentScore = 100;
int Game::mCurrentRound = 0;

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
std::unordered_set<uint64_t> Game::GetNextRoundInputs()
{
  std::unordered_set<uint64_t> Indecies;

  mCurrentRoundInputs.clear();

  while (mCurrentRoundInputs.size() < GetRoundSize())
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

      Indecies.insert(GetId(InputVariant));
    }
  }

  mCurrentScore = 100;

  return Indecies;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::SetNextRoundInputs(std::unordered_set<uint64_t>& Indecies)
{
  auto GetId = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetId(); }, InputVariant);};

  mCurrentRoundInputs.clear();

  for (auto& InputVariant : mInputs)
  {
    if (Indecies.count(GetId(InputVariant)))
    {
      mCurrentRoundInputs.emplace_back(InputVariant);
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Game::GetNextInputDisplay()
{
  auto GetId = [] (auto& InputVariant) {return std::visit([] (auto& Input) { return Input.GetId(); }, InputVariant);};

  auto GetNext = [this] { return *std::next(
    mCurrentRoundInputs.begin(),
    GetRandomInputIndex(mCurrentRoundInputs.size()));};

  auto Next = GetNext();

  if (moCurrentActiveVariant && GetId(moCurrentActiveVariant->get()) == GetId(Next.get()))
  {
    return Game::GetNextInputDisplay();
  }

  moCurrentActiveVariant = Next;

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

  return Bits.to_ullong();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint64_t Game::GetHardwareValue(uint64_t PiSerial) const
{
  std::bitset<64> Bits(std::numeric_limits<uint64_t>::max());

  for (const auto& Output : mOutputs)
  {
    if (Output.mPiSerial == PiSerial)
    {
      Bits[Output.mId] = Output.mCurrentState;
    }
  }

  return Bits.to_ullong();
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
