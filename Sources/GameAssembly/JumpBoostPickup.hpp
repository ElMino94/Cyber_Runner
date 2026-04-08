#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <JSON/json.hpp>

class JumpBoostPickup : public TerminaScript::ScriptableComponent
{
public:
    JumpBoostPickup() = default;
    JumpBoostPickup(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float deltaTime) override;
    void Inspect() override;
    void Serialize(nlohmann::json& out) const override;
    void Deserialize(const nlohmann::json& in) override;
    void OnTriggerEnter(Termina::Actor* other) override;

public:
    float multiplier = 1.5f;  // Multiplicateur du saut (ex: 1.5 = 50% plus haut)
    float duration = 5.0f;    // Durée en secondes
    bool IsTaken = false;

protected:
    Termina::Actor* m_Player = nullptr;

private:
    float m_RotationSpeed = 120.0f;
};