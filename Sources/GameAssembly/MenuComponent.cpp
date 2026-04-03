#include "MenuComponent.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Logger.hpp>

/*
* Called once
*/
void MenuComponent::Start()
{
    m_IsActive = true;
    m_PlayRequested = false;
    m_QuitRequested = false;

    TN_INFO("MenuComponent started");
}

/*
* Called every frame
*/
void MenuComponent::Update(float deltaTime)
{
    if (!m_IsActive)
        return;

    DrawMenu();
}

/*
* Draw menu
*/
void MenuComponent::DrawMenu()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize;

    ImVec2 size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(
        ImVec2(size.x * 0.5f, size.y * 0.5f),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );

    ImGui::Begin("Main Menu", nullptr, flags);

    ImGui::Text("Runner Game");
    ImGui::Separator();

    /*
    * Play button
    */
    if (ImGui::Button("Play", ImVec2(200.0f, 40.0f)))
    {
        TN_INFO("Play requested");

        m_PlayRequested = true;
        m_IsActive = false;
    }

    /*
    * Quit button
    */
    if (ImGui::Button("Quit", ImVec2(200.0f, 40.0f)))
    {
        TN_INFO("Quit requested");

        m_QuitRequested = true;
    }

    ImGui::End();
}

/*
* Debug
*/
void MenuComponent::Inspect()
{
    ImGui::Checkbox("Menu Active", &m_IsActive);
    ImGui::Text("Play Requested: %s", m_PlayRequested ? "true" : "false");
    ImGui::Text("Quit Requested: %s", m_QuitRequested ? "true" : "false");
}

/*
* Serialization
*/
void MenuComponent::Serialize(nlohmann::json& out) const
{
    out["isActive"] = m_IsActive;
}

/*
* Deserialization
*/
void MenuComponent::Deserialize(const nlohmann::json& in)
{
    if (in.contains("isActive"))
        m_IsActive = in["isActive"];
}