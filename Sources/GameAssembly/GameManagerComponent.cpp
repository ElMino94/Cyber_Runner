#include "GameManagerComponent.hpp"
#include "MenuComponent.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/World/WorldSystem.hpp>
#include <Termina/World/World.hpp>
#include <Termina/Core/Logger.hpp>

void GameManagerComponent::Start()
{
    m_GameStarted = false;
}

void GameManagerComponent::Update(float deltaTime)
{
    auto* worldSystem =
        Termina::Application::GetSystem<Termina::WorldSystem>();

    if (!worldSystem)
        return;

    Termina::World* world = worldSystem->GetCurrentWorld();
    if (!world)
        return;

    for (auto& actorPtr : world->GetActors())
    {
        Termina::Actor* actor = actorPtr.get();

        MenuComponent* menu = nullptr;

        for (auto* comp : actor->GetAllComponents())
        {
            menu = dynamic_cast<MenuComponent*>(comp);
            if (menu)
                break;
        }

        if (!menu)
            continue;

        if (menu->IsPlayRequested() && !m_GameStarted)
        {
            m_GameStarted = true;

            TN_INFO("Loading game world");

            worldSystem->LoadWorld("Assets/Worlds/Maps/map_teva1");

            return;
        }

        if (menu->IsQuitRequested())
        {
            TN_INFO("Quit requested");

            return;
        }
    }
}