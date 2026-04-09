#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>

using namespace TerminaScript;

class GameOverComponent : public ScriptableComponent
{
public:
    GameOverComponent() = default;
    GameOverComponent(Termina::Actor* owner)
        : ScriptableComponent(owner) {
    }

    void OnRender(float dt) override;

private:
    std::string m_MenuWorld = "Assets/Worlds/Maps/map_menu";
};