#include <HardwareInterface/I2c.h>
#include <HardwareInterface/Spi.h>
#include <HardwareInterface/Types.hpp>

#include <boost/property_tree/json_parser.hpp>

#include <fmt/format.h>

#include <bitset>
#include <iostream>
#include <optional>
#include <thread>

std::string gPiSerial;

using namespace std::literals::chrono_literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void testPin(size_t Index)
{
  fmt::print("Printing Pin {}\n", Index);
  std::bitset<64> Value(std::numeric_limits<uint64_t>::max());

  for (int i = 0; i < 10; ++i)
  {
    st::hw::setGPIOVal(Value.to_ullong());

    Value[Index] = !Value[Index];

    std::this_thread::sleep_for(250ms);
  }

  st::hw::setGPIOVal(std::numeric_limits<uint64_t>::max());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::optional<size_t> GetOutput()
{
  std::array<uint8_t, 48> StartAnalog;

  st::hw::adcReadFIFOAll(StartAnalog);

  for (int i = 0; i < 500; ++i)
  {
    std::array<uint8_t, 48> Analog;

    st::hw::adcReadFIFOAll(Analog);

    for (auto i = 0u; i < Analog.size(); ++i)
    {
      if (std::abs(static_cast<int>(Analog[i]) - static_cast<int>(StartAnalog[i])) > 150)
      {
        return i;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  fmt::print("Timeout reached. press s to skip or any other key to try again\n");

  char Input;

  std::cin >> Input;

  if (Input == 's')
  {
    return std::nullopt;
  }

  return GetOutput();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::property_tree::ptree GetMomentaryInput(uint64_t Led, uint64_t Pin)
{
  boost::property_tree::ptree Tree, Input, LedTree;

  Input.put("Type", "Momentary");

  Input.put("Id", Pin);

  Input.put("PiSerial", gPiSerial);

  std::string Data;

  Input.put("Label", "blank");

  fmt::print("Enter the Message:\n");

  std::cin.ignore();

  std::getline(std::cin, Data);

  Input.put("Message", Data);

  fmt::print("Dont touch the input\n");

  std::this_thread::sleep_for(1s);

  std::array<uint8_t, 48> Bits;

  st::hw::adcReadFIFOAll(Bits);

  Input.put("Default Value", static_cast<int>(Bits[Pin]));

  Tree.add_child("Input", Input);

  LedTree.put("Id", Led);

  LedTree.put("PiSerial", gPiSerial);

  LedTree.put("Input", Pin);

  Tree.add_child("Output", LedTree);

  return Tree;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::property_tree::ptree GetThreshold(uint64_t Pin)
{
  boost::property_tree::ptree Tree;

  std::string Data;

  fmt::print("Enter the Label:\n");

  std::cin.ignore();

  std::getline(std::cin, Data);

  Tree.put("Label", Data);

  std::array<uint8_t, 48> Analog;

  fmt::print("Enter any key when at start\n");

  std::getline(std::cin, Data);

  st::hw::adcReadFIFOAll(Analog);

  Tree.put("Start", Analog[Pin]);

  fmt::print("Enter any key when at stop\n");

  std::getline(std::cin, Data);

  st::hw::adcReadFIFOAll(Analog);

  Tree.put("Stop", Analog[Pin]);

  return Tree;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::property_tree::ptree GetAnalogInput(uint64_t Led, uint64_t Pin)
{
  boost::property_tree::ptree Tree, Input, LedTree;

  Input.put("Type", "Analog");

  Input.put("Id", Pin);

  Input.put("PiSerial", gPiSerial);

  std::string Data;

  fmt::print("Enter the Label:\n");

  std::cin.ignore();

  std::getline(std::cin, Data);

  Input.put("Label", Data);

  fmt::print("Enter how many thresholds:\n");

  int NumberOfThresholds;

  std::cin >> NumberOfThresholds;

  for (int i = 0; i < NumberOfThresholds; ++i)
  {
    Input.add_child("Threshold", GetThreshold(Pin));
  }

  Tree.add_child("Input", Input);

  //Led
  LedTree.put("Id", Led);

  LedTree.put("PiSerial", gPiSerial);

  LedTree.put("Input", Pin);

  Tree.add_child("Output", LedTree);

  return Tree;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::property_tree::ptree GetDigitalInput(uint64_t Led, uint64_t Pin)
{
  boost::property_tree::ptree Tree, Input, LedTree;

  Input.put("Type", "Digital");

  Input.put("Id", Pin);

  Input.put("PiSerial", gPiSerial);

  std::string Data;

  fmt::print("Enter the Label:\n");

  std::cin.ignore(); // clear newline

  std::getline(std::cin, Data);

  Input.put("Label", Data);

  fmt::print("Enter Label for the Current State:\n");

  std::getline(std::cin, Data);

  auto GetLabel = [] (bool State) { return State ? "On Label" : "Off Label"; };

  std::array<uint8_t, 48> Analog;

  st::hw::adcReadFIFOAll(Analog);

  Input.put(GetLabel(Analog[Pin] > 128), Data);

  fmt::print("Flip state\n");

  std::array<uint8_t, 48> FlippedAnalog;

  bool isFlipped = false;

  while (!isFlipped)
  {
    st::hw::adcReadFIFOAll(FlippedAnalog);

    if (std::abs(static_cast<int>(Analog[Pin]) - static_cast<int>(FlippedAnalog[Pin])) > 150)
    {
      isFlipped = true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  fmt::print("Enter Label for the Current State:\n");

  std::getline(std::cin, Data);

  Input.put("Off Label", Data);

  Tree.add_child("Input", Input);

  LedTree.put("Id", Led);

  LedTree.put("PiSerial", gPiSerial);

  LedTree.put("Input", Pin);

  Tree.add_child("Output", LedTree);

  return Tree;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  gPiSerial = fmt::format("00000000{:x}", GetSerialNumber());

  fmt::print("Raspberry Pi Serial = {}\n", gPiSerial);

  fmt::print("Setting up Digital Output Pins\n");

  st::hw::setAllGPIOOutput();

  std::vector<size_t> Leds;

  char Input;

  fmt::print("argc = {}\n", argc);

  if (argc == 2)
  {
    auto StartingIndex = std::atoi(argv[1]);

    std::vector<int> HardCodedLeds = {
      2,3,4,5,6,10,12,13,14,15,16,17,22,25,26,27,28,29,30,31,32,33,37,38,39};


    for (size_t i = StartingIndex; i < HardCodedLeds.size();++i)
    {
      Leds.emplace_back(HardCodedLeds[i]);
    }
  }
  else
  {
    for (auto i = 0; i < 48; ++i)
    {
      testPin(i);

      fmt::print("Did something happen? Press y if something did. Press r to retry\n");

      std::cin >> Input;

      if (Input == 'y')
      {
        fmt::print("woot\n");

        Leds.emplace_back(i);
      }
      else if (Input == 'r')
      {
        --i;
      }

    }
      fmt::print("found {} of 25\n", Leds.size());
  }

  std::bitset<64> Bits(std::numeric_limits<uint64_t>::max());

  for (const auto Led : Leds)
  {
    Bits[Led] = false;
  }

  st::hw::setGPIODir(Bits.to_ullong());

  boost::property_tree::ptree Tree;

  for (const auto Led : Leds)
  {
    fmt::print("{},", Led);
  }

  fmt::print("\n");

  for (const auto Led : Leds)
  {
    std::bitset<64> Value(std::numeric_limits<uint64_t>::max());

    Value[Led] = false;

    st::hw::setGPIOVal(Value.to_ullong());

    fmt::print("Switch State of associated dingle\n");

    auto oDevice = GetOutput();

    if (oDevice)
    {
      auto SubTree = [&Led, &oDevice, &Input]
        {
          fmt::print("found corresponding input!\n type m for Momentary or d for Digital or a for Analog\n");

          std::cin >> Input;

          while(true)
          {
            if (Input == 'd')
            {
              return GetDigitalInput(Led, *oDevice);
            }
            else if (Input == 'm')
            {
              return GetMomentaryInput(Led, *oDevice);
            }
            else if (Input == 'a')
            {
              return GetAnalogInput(Led, *oDevice);
            }
          }
        }();

      for (const auto& [Label, IoTree] : SubTree)
      {
        Tree.add_child(Label, IoTree);
      }

      boost::property_tree::write_json("Config.json", Tree);
    }

  }

  boost::property_tree::write_json("Config.json", Tree);

  boost::property_tree::write_json(std::cout, Tree);

}
