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

public:
    float multiplier = 2.0f;
    float duration = 5.0f;

private:
    float m_RotationSpeed = 120.0f;
};