#pragma once
#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <vector>
using namespace TerminaScript;

class SideScrolling : public TerminaScript::ScriptableComponent
{
public:
    SideScrolling() = default;
    SideScrolling(Termina::Actor* owner) : TerminaScript::ScriptableComponent(owner) {}

    void Start()  override;
    void Update(float deltaTime) override;

    void CorridorSideScrolling();
	void CheckIfNeedReplace();

protected:
    Termina::Actor* m_Player = nullptr;
	std::vector<Termina::Actor*> m_Segments;
};
