#include "GameOverComponent.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Application.hpp>
#include <Termina/World/WorldSystem.hpp>

void GameOverComponent::OnRender(float dt)
{
    ImGuiIO& io = ImGui::GetIO();

    float width = io.DisplaySize.x;
    float height = io.DisplaySize.y;

    // Background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("Background", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoInputs);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    drawList->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 255));

    ImGui::End();

    // Title
    ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.3f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("GameOver", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::TextColored(ImVec4(1, 0, 0, 1), "GAME OVER");

    ImGui::End();

    // Buttons
    ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.7f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Menu", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("Retry", ImVec2(200, 50)))
    {
        auto* worldSystem = Termina::Application::GetSystem<Termina::WorldSystem>();
        if (worldSystem)
        {
            worldSystem->LoadWorld("Assets/Worlds/Maps/map_teva1");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Back to Menu", ImVec2(200, 50)))
    {
        auto* worldSystem = Termina::Application::GetSystem<Termina::WorldSystem>();
        if (worldSystem)
        {
            worldSystem->LoadWorld("Assets/Worlds/Maps/map_menu");
        }
    }

    ImGui::End();
}