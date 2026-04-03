#pragma once

#include <Termina/Scripting/API/ScriptingAPI.hpp>

class GameManagerComponent : public TerminaScript::ScriptableComponent
{
public:
    GameManagerComponent() = default;
    GameManagerComponent(Termina::Actor* owner)
        : ScriptableComponent(owner) {
    }

    void Update(float deltaTime) override;
};