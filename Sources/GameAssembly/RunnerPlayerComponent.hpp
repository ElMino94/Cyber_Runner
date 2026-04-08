#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <JSON/json.hpp>

class RunnerPlayerComponent : public TerminaScript::ScriptableComponent
{
public:
    RunnerPlayerComponent() = default;
    RunnerPlayerComponent(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float deltaTime) override;
    void Inspect() override;
    void Serialize(nlohmann::json& out) const override;
    void Deserialize(const nlohmann::json& in) override;
    void OnCollisionEnter(Termina::Actor* other) override;

public:
    void ActivateJumpBoost(float multiplier, float duration);

private:
    void HandleInput();
    void MoveForward(float deltaTime);
    void MoveToTargetLane(float deltaTime);
    void UpdateJump(float deltaTime);
    void UpdateJumpBoost(float deltaTime);
    void KillPlayer();

private:
    float m_ForwardSpeed = 10.0f;
    float m_LaneOffset = 2.5f;
    float m_LaneChangeSpeed = 14.0f;

    float m_BaseJumpForce = 9.0f;
    float m_JumpForce = 9.0f;
    float m_Gravity = 25.0f;
    float m_GroundY = 0.0f;

    float m_JumpBoostMultiplier = 1.5f;
    float m_JumpBoostDuration = 5.0f;
    float m_JumpBoostTimer = 0.0f;

    int   m_CurrentLane = 0;   // -1 gauche, 0 centre, 1 droite
    float m_VerticalVelocity = 0.0f;

    bool  m_IsGrounded = true;
    bool  m_IsDead = false;
    bool  m_HasJumpBoost = false;

    int m_Score = 0;

    glm::vec3 m_StartPosition = glm::vec3(0.0f);
};