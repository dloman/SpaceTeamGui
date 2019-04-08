#include "Game.hpp"
#include <SpaceTeam/Success.hpp>
#include <Utility/Visitor.hpp>
#include <boost/property_tree/ptree.hpp>
#include <random>

using st::Game;

namespace
{
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
  std::vector<st::InputVariant> GetInputs(boost::property_tree::ptree& Tree)
  {
    std::vector<st::InputVariant> Inputs;

    for (const auto& [Label, SubTree]: Tree)
    {
      Inputs.emplace_back(GetInput(SubTree));
    }

    return Inputs;
  }
}

int Game::mCurrentScore = 100;
int Game::mCurrentRound = 100;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Game::Game(boost::property_tree::ptree& Tree)
: mInputs(GetInputs(Tree)),
  moCurrentActiveVariant(std::nullopt),
  mLastResetTime(std::chrono::milliseconds(0))
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Game::GetRoundInputsSize() const
{
  return 2;
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
double Game::GetCurrentState(const InputVariant& Input)
{
  return 0.0; //TODO
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Game::UpdateCurrentState()
{
  for (const auto& Input : mInputs)
  {
    auto CurrentState = GetCurrentState(Input);

    return std::visit(st::Visitor{
      [CurrentState] (st::Analog& Input)
      {
        return Input.SetCurrentState(CurrentState);
      },
      [CurrentState] (auto& Input)
      {
        return Input.SetCurrentState(std::abs(CurrentState - 0.0) > 0.1);
      }},
      moCurrentActiveVariant->get());
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
