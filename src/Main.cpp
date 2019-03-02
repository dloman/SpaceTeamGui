#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <Random/Random.hpp>
#include <chrono>

using namespace std::literals;

std::vector<std::string> gMessages
{
  "Enable poopshoot suction device",
  "Mike sux cox n dix",
  "Swap poop deck",
  "Wish your mother a happy birthday",
  "Disable stranglet diffuser",
  "Chug some dingle",
  "dangle your dongle",
  "do something really really really cool",
  "do something really really really cool, but like seriously",
  "this one has a lot of words. like way more then it should but thats ok. actually this is kindof stupid there really is no reason for this many words."
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::array<float, 8> GetData()
{
  return {
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>(),
    dl::random::GetUniform<float>()
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
//------------------------------------------------------------------------------
std::string GetRandomTextToDisplay()
{
  static std::random_device RandomDevice;
  static std::mt19937 Generator(RandomDevice());
  std::uniform_int_distribution<> Distribution(0, gMessages.size());;

  return gMessages[Distribution(Generator)];
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

auto gTextToDisplay = GetRandomTextToDisplay();
std::string gCurrentText;
auto gTextUpdate = std::chrono::system_clock::now();

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

#include <iostream>
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

  ImGui::Button("Comply with the following instructions immediatly to avoid irreversible death:");


  if (gCurrentText.size() != gTextToDisplay.size())
  {
    gCurrentText = gTextToDisplay.substr(0, gCurrentText.size() + 1);

    gTextUpdate = std::chrono::system_clock::now();

  }
  else if (std::chrono::system_clock::now() - gTextUpdate > 20s)
  {
    gCurrentText = "";

    gTextToDisplay = GetRandomTextToDisplay();
  }

    ImGui::TextWrapped(gCurrentText.c_str());

    if (gCurrentText.size() == gTextToDisplay.size())
    {
      float Progress = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - gTextUpdate).count() / 20.0;

      ImGui::ProgressBar(Progress, {-1,0}, "Time Remaining");
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
int main()
{
  sf::RenderWindow window(sf::VideoMode(1280, 800), "H4ckerSp4ce t3AM");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

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

    DrawDataPanel();
    DrawTaskPanel();
    DrawGraphPanel();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}
