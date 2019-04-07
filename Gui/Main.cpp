#include <imgui.h>
#include <imgui-SFML.h>

#include <SpaceTeam/Game.hpp>
#include <SpaceTeam/Success.hpp>
#include <Utility/Random.hpp>

#include <Tcp/Client.hpp>
#include <Robot/PacketEncoder.hpp>
#include <Robot/PacketDecoder.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <fmt/format.h>
#include <chrono>

using namespace std::literals;

std::unique_ptr<dl::tcp::Client<dl::tcp::Session>> gpClient;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::array<float, 8> GetData()
{
  return {
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>(),
    st::random::GetUniform<float>()
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
float GetRandomTemp()
{
  static std::random_device RandomDevice;
  static std::mt19937 Generator(RandomDevice());
  static std::uniform_real_distribution<float> Distribution(0,1);;

  return Distribution(Generator);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::array<float, 11> GetRandomHorizontalData()
{
  return {
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
    GetRandomTemp(),
  };
}

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------
auto gHistogram1Data = GetData();
auto gHistogram2Data = GetData();
auto gLine1Data = GetData();
auto gLine2Data = GetData();
auto gLine3Data = GetData();
auto gFastUpdate = std::chrono::system_clock::now();
auto gSlowUpdate = std::chrono::system_clock::now();
auto gCoreTemp = GetRandomTemp();
auto gSystemTemp = GetRandomTemp();
auto gCabinTemp = GetRandomTemp();
auto gToiletSeatTemp = GetRandomTemp();
auto gDingler = GetRandomTemp() > .5f;
auto gSelfDestruct = GetRandomTemp() > .5f;
auto gRecycleBin = GetRandomTemp() > .5f;

auto gHorizontalData = GetRandomHorizontalData();
auto gHorizontalUpdate = std::chrono::system_clock::now();

std::string gTextToDisplay("");
std::string gCurrentText("");
auto gTextUpdate = std::chrono::system_clock::now();
ImFont* gpFont20;
ImFont* gpFont30;

static const ImVec2 PlotSize = {440, 145};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DrawDataPanel()
{
  ImGui::Begin(
    "Data",
    nullptr,
    ImVec2(0, 0), 1.0f,
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

  ImGui::SetWindowPos({0, 0});
  ImGui::SetWindowSize({455, 800});
  ImGui::Button("Flux Endonglanator", {-1,0});

  if (std::chrono::system_clock::now() - gFastUpdate > 1s)
  {
    gLine1Data = GetData();
    gLine2Data = GetData();
    gLine3Data = GetData();
    gHistogram1Data = GetData();
    gHistogram2Data = GetData();
    gDingler = GetRandomTemp() > .5f;
    gSelfDestruct = GetRandomTemp() > .5f;
    gRecycleBin = GetRandomTemp() > .5f;
    gFastUpdate = std::chrono::system_clock::now();
  }

  if (std::chrono::system_clock::now() - gSlowUpdate > 3s)
  {
    gCoreTemp = GetRandomTemp();
    gCabinTemp = GetRandomTemp();
    gSystemTemp = GetRandomTemp();
    gToiletSeatTemp = GetRandomTemp();

    gSlowUpdate = std::chrono::system_clock::now();
  }

  ImGui::PlotHistogram("", gHistogram1Data.data(), gHistogram1Data.size(), 0, NULL, 0.0f, 1.0f, PlotSize);
  ImGui::ProgressBar(gCoreTemp, {-1,0},"Core °C");

  ImGui::ProgressBar(gSystemTemp, {-1,0},"System °C");

  ImGui::ProgressBar(gCabinTemp, {-1,0},"Cabin °C");

  ImGui::ProgressBar(gToiletSeatTemp, {-1,0},"Toilet Seat °C");

  ImGui::Button("Chooch Inverters", {-1,0});
  ImGui::RadioButton("Dingler Enabled", gDingler);
  ImGui::SameLine();
  ImGui::Checkbox("Self Destruct Engaged", &gSelfDestruct);
  ImGui::SameLine();
  ImGui::RadioButton("Recyle Bin Full", gRecycleBin);

  ImGui::PlotLines("", gLine1Data.data(), gLine1Data.size(), 0, NULL, 0.0f, 1.0f, PlotSize);
  ImGui::PlotHistogram("", gHistogram2Data.data(), gHistogram2Data.size(), 0, NULL, 0.0f, 1.0f, PlotSize);
  ImGui::PlotLines("", gLine2Data.data(), gLine2Data.size(), 0, NULL, 0.0f, 1.0f, PlotSize);

  ImGui::End();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ResetActionText(const std::string& Text)
{
  gCurrentText = "";

  gTextToDisplay = Text;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendSuccess(bool Success)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DrawTaskPanel()
{
  ImGui::Begin(
    "Tasking",
    nullptr,
    ImVec2(0, 0),
    1.0f,
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

  ImGui::SetWindowSize({820, 500});

  ImGui::SetWindowPos({460, 0});

  // Load Fonts
  ImGui::PushFont(gpFont20);

  ImGui::Button("Comply with the following instructions to avoid irreversible death:");

  ImGui::Text("\n");
  ImGui::PopFont();

  ImGui::PushFont(gpFont30);

  if (gCurrentText.size() != gTextToDisplay.size())
  {
    gCurrentText = gTextToDisplay.substr(0, gCurrentText.size() + 1);

    gTextUpdate = std::chrono::system_clock::now();

  }
  else if (std::chrono::system_clock::now() - gTextUpdate > 20s)
  {
    SendSuccess(false);

    ResetActionText("");
  }

  ImGui::TextWrapped(gCurrentText.c_str());

  ImGui::PopFont();

  if (gCurrentText.size() == gTextToDisplay.size())
  {
    using namespace std::chrono;
    float Progress = duration_cast<seconds>(
      system_clock::now() - gTextUpdate).count() / 20.0;

    ImGui::SetCursorPos({0, 460});

    ImGui::PushFont(gpFont30);

    ImGui::ProgressBar(
      Progress,
      {-1,30},
      (std::to_string(static_cast<int> (20 - 20*Progress)) +"s Remaining").c_str());

    ImGui::PopFont();
  }


  ImGui::End();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DrawGraphPanel()
{
  ImGui::Begin(
    "Engine Flux Power Output",
    nullptr,
    ImVec2(610, 0),
    1.0f,
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

  ImGui::SetWindowPos({460, 505});
  ImGui::SetWindowSize({820, 290});

  if (std::chrono::system_clock::now() - gHorizontalUpdate > 1s)
  {
    gHorizontalData = GetRandomHorizontalData();

    gHorizontalUpdate = std::chrono::system_clock::now();
  }

  for (const auto& Data : gHorizontalData)
  {
    ImGui::ProgressBar(Data, {-1,0},"");
  }
  ImGui::End();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SendState(const std::string& State)
{
  gpClient->Write(State);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string GetState()
{
  std::string State;

  //TODO
  return State;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OnRx(const std::string& Bytes)
{
  std::istringstream Stream(Bytes);

  boost::property_tree::ptree Tree;

  boost::property_tree::read_json(Stream, Tree);

  if (const auto oText = Tree.get_optional<std::string>("reset"))
  {
    ResetActionText(*oText);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  gpClient = std::make_unique<dl::tcp::Client<dl::tcp::Session>>(
    dl::tcp::ClientSettings<dl::tcp::Session>{
      .mOnRxCallback = OnRx,
      .mConnectionCallback = [] (const auto&) { fmt::print("connected\n");},
      .mConnectionErrorCallback = ResetActionText});

  sf::RenderWindow window(sf::VideoMode(1280, 800), "H4ckerSp4ce t3AM");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  gpFont20 = io.Fonts->AddFontFromFileTTF("/home/dloman/ProggyClean.ttf", 20.f);
  gpFont30 = io.Fonts->AddFontFromFileTTF("/home/dloman/ProggyClean.ttf", 30.f);

  ImGui::SFML::UpdateFontTexture();

  sf::Clock deltaClock;

  while (window.isOpen())
  {
    SendState(GetState());

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

    DrawDataPanel();
    DrawTaskPanel();
    DrawGraphPanel();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}
