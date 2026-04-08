#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <JSON/json.hpp>

class RunnerGameManager : public TerminaScript::ScriptableComponent
{
public:
    RunnerGameManager() = default;
    RunnerGameManager(Termina::Actor* owner)
        : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float deltaTime) override;
    void Inspect() override;
    void Serialize(nlohmann::json& out) const override;
    void Deserialize(const nlohmann::json& in) override;

    void AddScore(int amount);
    int GetScore() const { return m_Score; }

private:
    void DrawHUD();

private:
    int   m_Score = 0;
    float m_ScoreTimer = 0.0f;
    float m_ScoreRate = 10.0f;

    bool  m_GameOver = false;
};