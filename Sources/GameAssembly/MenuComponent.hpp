#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>

using namespace TerminaScript;

class MenuComponent : public ScriptableComponent
{
public:
    MenuComponent() = default;
    MenuComponent(Termina::Actor* owner)
        : ScriptableComponent(owner) {
    }

    void OnRender(float dt) override;

    bool IsPlayRequested() const { return m_PlayRequested; }
    bool IsQuitRequested() const { return m_QuitRequested; }

    void ResetRequests()
    {
        m_PlayRequested = false;
        m_QuitRequested = false;
    }

private:
    std::string m_WorldToLoad = "Assets/Worlds/Maps/map_teva1";

    bool m_PlayRequested = false;
    bool m_QuitRequested = false;
};