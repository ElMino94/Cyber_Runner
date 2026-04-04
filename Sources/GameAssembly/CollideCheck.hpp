#pragma once
#include <Termina/Scripting/API/ScriptingAPI.hpp>

using namespace TerminaScript;

class CollideStatus : public TerminaScript::ScriptableComponent
{
public:
    CollideStatus() = default;
    CollideStatus(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start()  override;
    void Update(float deltaTime) override;
    void OnTriggerEnter(Termina::Actor* other) override;

protected:
    Termina::Actor* m_Player = nullptr;
};