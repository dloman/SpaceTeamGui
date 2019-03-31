#include "Game.hpp"
#include <SpaceTeam/Success.hpp>
#include <Utility/Visitor.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <random>

using st::Game;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::vector<st::InputVariant> GetInputs(const std::string& InputFile)
{
  std::vector<st::InputVariant> Inputs;

  boost::property_tree::ptree Tree;

  boost::property_tree::read_json(InputFile, Tree);

  for (const auto& [Label, SubTree]: Tree)
  {
    Inputs.emplace_back(GetInput(SubTree));
  }

  return Inputs;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Game::Game()
: mInputs(GetInputs("Setup.json")),
  moCurrentActiveVariant(std::nullopt)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Game::GetNextInputDisplay()
{
  static std::random_device RandomDevice;
  static std::mt19937 Generator(RandomDevice());
  std::uniform_int_distribution<> Distribution(0, mInputs.size() - 1);;

  moCurrentActiveVariant = mInputs[Distribution(Generator)];

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

  for(auto& InputVariant : mInputs)
  {
    std::visit(
      [&Success] (auto& Input) { return Input.IsCorrect(Success); },
      InputVariant);
  }

  return Success;
}
