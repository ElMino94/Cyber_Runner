#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <JSON/json.hpp>

class MenuComponent : public TerminaScript::ScriptableComponent
{
public:
    MenuComponent() = default;
    MenuComponent(Termina::Actor* owner)
        : TerminaScript::ScriptableComponent(owner) {}

    void Start() override;
    void Update(float deltaTime) override;
    void Inspect() override;

    void Serialize(nlohmann::json& out) const override;
    void Deserialize(const nlohmann::json& in) override;

    bool IsPlayRequested() const { return m_PlayRequested; }
    bool IsQuitRequested() const { return m_QuitRequested; }

private:
    void DrawMenu();

private:
    bool m_IsActive = true;

    bool m_PlayRequested = false;
    bool m_QuitRequested = false;
};