#include "MenuComponent.hpp"

using namespace TerminaScript;

class GameManagerComponent : public ScriptableComponent
{
public:
    GameManagerComponent() = default;
    GameManagerComponent(Termina::Actor* owner)
        : ScriptableComponent(owner) {
    }

    void Start() override;
    void Update(float deltaTime) override;

public:
    // Public members

private:
    // Private functions
    void HandleMenu(MenuComponent* menu);

private:
    // Private members
    bool m_GameStarted = false;
};