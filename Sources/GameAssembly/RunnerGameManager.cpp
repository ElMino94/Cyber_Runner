#include "RunnerGameManager.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Logger.hpp>

void RunnerGameManager::Start()
{
    m_Score = 0;
    m_ScoreTimer = 0.0f;
    m_GameOver = false;

    TN_INFO("RunnerGameManager started");
}

void RunnerGameManager::Update(float deltaTime)
{
    if (!m_GameOver)
    {
        m_ScoreTimer += deltaTime;

        const float pointsToAdd = m_ScoreTimer * m_ScoreRate;
        const int wholePoints = static_cast<int>(pointsToAdd);

        if (wholePoints > 0)
        {
            m_Score += wholePoints;
            m_ScoreTimer -= static_cast<float>(wholePoints) / m_ScoreRate;
        }
    }

    DrawHUD();
}

void RunnerGameManager::AddScore(int amount)
{
    m_Score += amount;
}

void RunnerGameManager::DrawHUD()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (ImGui::Begin("RunnerHUD", nullptr, flags))
    {
        ImGui::Text("Score: %d", m_Score);
    }
    ImGui::End();
}

void RunnerGameManager::Inspect()
{
    ImGui::DragFloat("Score Rate", &m_ScoreRate, 0.1f, 0.0f, 1000.0f);
    ImGui::Text("Current Score: %d", m_Score);
    ImGui::Checkbox("Game Over", &m_GameOver);

    if (ImGui::Button("Add 100 Score"))
        AddScore(100);

    if (ImGui::Button("Reset Score"))
    {
        m_Score = 0;
        m_ScoreTimer = 0.0f;
    }
}

void RunnerGameManager::Serialize(nlohmann::json& out) const
{
    out["score"] = m_Score;
    out["scoreRate"] = m_ScoreRate;
    out["gameOver"] = m_GameOver;
}

void RunnerGameManager::Deserialize(const nlohmann::json& in)
{
    if (in.contains("score")) m_Score = in["score"];
    if (in.contains("scoreRate")) m_ScoreRate = in["scoreRate"];
    if (in.contains("gameOver")) m_GameOver = in["gameOver"];
}