#pragma once
#include <Termina/Scripting/API/ScriptingAPI.hpp>

using namespace TerminaScript;

class Collectibles : public TerminaScript::ScriptableComponent
{
public:
    Collectibles() = default;
    Collectibles(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start()  override;
    void Update(float deltaTime) override;

    int value = 1;


    bool IsCollected() const { return m_isCollected; }
    void Collect() { m_isCollected = true; }

private:
    bool m_isCollected = false;


    

};
