#pragma once
#include <Termina/Scripting/API/ScriptingAPI.hpp>
using namespace TerminaScript;

class RunnerGameManager;

class Collectibles : public TerminaScript::ScriptableComponent
{
public:
    Collectibles() = default;
    Collectibles(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start()  override;
    void Update(float deltaTime) override;

    int value = 20;

    bool IsCollected() const { return m_isCollected; }
    void Collect() { m_isCollected = true; }
    void OnTriggerEnter(Termina::Actor* other) override;

private:
    bool m_isCollected = false;
    RunnerGameManager* m_GameManager = nullptr;

protected:
    Termina::Actor* m_Player = nullptr;
};
