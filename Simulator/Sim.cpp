#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Update.hpp>
#include <Utility/Visitor.hpp>
#include <Utility/XmlAssembler.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <Tcp/Client.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fmt/format.h>
#include <bitset>
#include <chrono>

using namespace std::literals;

int gCurrentIndex = 0;
st::XmlAssembler gXmlPacketAssembler;

std::unique_ptr<dl::tcp::Client<dl::tcp::Session>> gpClient;

ImFont* gpFont15;
ImFont* gpFont20;
ImFont* gpFont30;

std::vector<std::string> gSerials;
uint64_t mCurrentSerial;

namespace
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static auto VectorGetter = [](void* pInputVector, int Index, const char** pOutputText)
  {
    auto& Vector = *static_cast<std::vector<std::string>*>(pInputVector);
    if (Index < 0 || Index >= static_cast<int>(Vector.size()))
    {
      return false;
    }
    *pOutputText = Vector.at(Index).c_str();

    return true;
  };


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  bool MakeCombo(const char* Label, int& CurrentIndex, std::vector<std::string>& Values)
  {
    if (Values.empty())
    {
      return false;
    }
    return ImGui::Combo(
      Label,
      &CurrentIndex,
      VectorGetter,
      static_cast<void*>(&Values),
      Values.size());
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  unsigned GetLedId(unsigned Id, const st::Game& Game)
  {
    for(const auto& Output : Game.GetOutputs())
    {
      if (Output.mInput == Id)
      {
        return Output.mId;
      }
    }
    throw std::logic_error("no output found");
  }

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
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace sim
{
  struct Analog
  {
    const uint64_t mPiSerial;

    const std::string mLabel;

    const unsigned mId;

    const unsigned mLedId;

    uint8_t mState;

    bool mIsActive;

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Draw()
    {
      if (mPiSerial != GetSerial(gSerials[gCurrentIndex]))
      {
        return;
      }

      ImGui::Checkbox("", &mIsActive);

      ImGui::SameLine();

      ImGui::ProgressBar(mState, {-1,0}, mLabel.c_str());
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    st::Update GetUpdate()
    {
      return st::Update{.mPiSerial=mPiSerial, .mId=mId, .mValue = 0};
    }
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  struct Digital
  {
    const uint64_t mPiSerial;

    const std::string mLabel;

    const std::string mOnLabel;

    const std::string mOffLabel;

    const unsigned mId;

    const unsigned mLedId;

    int mState;

    bool mIsActive;

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Draw()
    {
      if (mPiSerial != GetSerial(gSerials[gCurrentIndex]))
      {
        return;
      }

      ImGui::Checkbox("", &mIsActive);

      ImGui::SameLine();

      ImGui::Text(mLabel.c_str());

      ImGui::SameLine();

      ImGui::RadioButton(mOnLabel.c_str(), &mState, 0);

      ImGui::SameLine();

      ImGui::RadioButton(mOffLabel.c_str(), &mState, 1);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    st::Update GetUpdate()
    {
      return st::Update{.mPiSerial=mPiSerial, .mId=mId, .mValue = 0};
    }
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  struct Momentary
  {
    const uint64_t mPiSerial;

    const std::string mMessage;

    const unsigned mId;

    const unsigned mLedId;

    const bool mDefaultValue;

    bool mState;

    bool mIsActive;

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Draw()
    {
      if (mPiSerial != GetSerial(gSerials[gCurrentIndex]))
      {
        return;
      }

      ImGui::Checkbox("", &mIsActive);

      ImGui::SameLine();

      mState = ImGui::Button(mMessage.c_str(), {-1,0}) ? !mDefaultValue : mDefaultValue;
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    st::Update GetUpdate()
    {
      return st::Update{.mPiSerial=mPiSerial, .mId=mId, .mValue = 0};
    }
  };
}

using DrawVariant = std::variant<sim::Analog, sim::Digital, sim::Momentary>;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DrawPanel(std::vector<DrawVariant>& Things)
{
  ImGui::Begin(
    "Data",
    nullptr,
    ImVec2(0, 0), 1.0f,
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

  ImGui::SetWindowPos({0, 0});
  ImGui::SetWindowSize({1600, 800});

  MakeCombo("Panel Selector", gCurrentIndex, gSerials);

  for(auto& Thing : Things)
  {
    std::visit([] (auto& Input) { Input.Draw(); }, Thing);
  }

  ImGui::End();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string SerializeAnalog(const st::Analog& Analog)
{
  return "";
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string SerializeDigital(std::bitset<64>& DigitalOutput)
{
  std::stringstream Stream;

  const uint8_t Digital = 0xFF;

  Stream.write(reinterpret_cast<const char*>(&Digital), 1);

  const auto Serial = GetSerial(gSerials[gCurrentIndex]);

  Stream.write(reinterpret_cast<const char*>(&Serial), 8);

  const uint64_t Data = DigitalOutput.to_ullong();

  Stream.write(reinterpret_cast<const char*>(&Data), 8);

  return Stream.str();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendState(std::vector<DrawVariant>& Things)
{
  std::string Bytes;

  std::bitset<64> DigitalOutput(std::numeric_limits<uint64_t>::max());

  for (auto& DrawVariant : Things)
  {
    std::visit(st::Visitor{
      [](sim::Analog& Input)
      {
        if (Input.mPiSerial != GetSerial(gSerials[gCurrentIndex]))
        {
          return;
        }
      },
      [&DigitalOutput](auto& Input)
      {
        if (Input.mPiSerial != GetSerial(gSerials[gCurrentIndex]))
        {
          return;
        }

        DigitalOutput[Input.mId] = Input.mState;
      }},
      DrawVariant);
  }

  gpClient->Write(SerializeDigital(DigitalOutput));
  fmt::print("output = {} \n", DigitalOutput.to_string());

  //gpClient->Write(SerializeAnalog());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  boost::property_tree::ptree Tree;

  boost::property_tree::read_json("Setup.json", Tree);

  st::Game Game(Tree);

  for (const auto PiSerial : Game.GetPiSerials())
  {
    gSerials.emplace_back(fmt::format("{:x}", PiSerial));
  }

  std::vector<DrawVariant> Things;

  for(auto& InputVariant : Game.GetInputs())
  {
    std::visit(st::Visitor{
      [&Things, &Game] (const st::Analog& Input)
      {
        Things.emplace_back(sim::Analog{
          .mPiSerial=Input.GetPiSerial(),
          .mLabel=Input.GetLabel(),
          .mId=Input.GetId(),
          .mLedId = GetLedId(Input.GetId(), Game),
          .mState = 0,
          .mIsActive = false});
      },
      [&Things, &Game] (const st::Digital& Input)
      {
        Things.emplace_back(sim::Digital{
          .mPiSerial = Input.GetPiSerial(),
          .mLabel = Input.GetLabel(),
          .mOnLabel = Input.GetOnLabel(),
          .mOffLabel = Input.GetOffLabel(),
          .mId = Input.GetId(),
          .mLedId = GetLedId(Input.GetId(), Game),
          .mState = 0,
          .mIsActive = false});
      },
      [&Things, &Game] (const st::Momentary& Input)
      {
        Things.emplace_back(sim::Momentary{
          .mPiSerial = Input.GetPiSerial(),
          .mMessage = Input.GetMessage(),
          .mId = Input.GetId(),
          .mLedId = GetLedId(Input.GetId(), Game),
          .mDefaultValue = Input.GetDefaultValue(),
          .mState = Input.GetDefaultValue(),
          .mIsActive = false});
      }},
      InputVariant);
  }

  //gXmlPacketAssembler.GetSignalPacket().Connect(
  //[] (const auto& Bytes) { OnXmlPacket(Bytes); });

  const auto Hostname = [&]
    {
      if(argc == 2)
      {
        return std::string(argv[1]);
      }
      return std::string("localhost");
    }();

  gpClient = std::make_unique<dl::tcp::Client<dl::tcp::Session>>(
    dl::tcp::ClientSettings<dl::tcp::Session>{
      .mHostname = Hostname,
      //.mOnRxCallback = OnRx,
      .mConnectionCallback = [] (const auto&) { fmt::print("connected\n");}});
  //.mConnectionErrorCallback = OnError});

  sf::RenderWindow window(sf::VideoMode(1610, 810), "H4ckerSp4ce t3AM");
  window.setFramerateLimit(30);
  ImGui::SFML::Init(window);

  window.setMouseCursorVisible(true);

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

#ifdef ENABLE_HARDWARE
  gpFont15 = io.Fonts->AddFontFromFileTTF("/home/pi/ProggyClean.ttf", 15.f);
  gpFont20 = io.Fonts->AddFontFromFileTTF("/home/pi/ProggyClean.ttf", 20.f);
  gpFont30 = io.Fonts->AddFontFromFileTTF("/home/pi/ProggyClean.ttf", 30.f);
#else
  gpFont15 = io.Fonts->AddFontFromFileTTF("/home/dloman/ProggyClean.ttf", 15.f);
  gpFont20 = io.Fonts->AddFontFromFileTTF("/home/dloman/ProggyClean.ttf", 20.f);
  gpFont30 = io.Fonts->AddFontFromFileTTF("/home/dloman/ProggyClean.ttf", 30.f);
#endif

  ImGui::SFML::UpdateFontTexture();

  sf::Clock deltaClock;


  while (window.isOpen())
  {
    sf::Event event;

    while (window.pollEvent(event))
    {
      ImGui::SFML::ProcessEvent(event);

      if (event.type == sf::Event::Closed)
      {
        window.close();
      }
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    ImGui::PushFont(gpFont30);

    DrawPanel(Things);

    ImGui::PopFont();

    SendState(Things);

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
  }


  ImGui::SFML::Shutdown();
}
