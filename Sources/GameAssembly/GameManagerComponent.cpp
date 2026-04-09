#include "GameManagerComponent.hpp"
#include "MenuComponent.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/World/WorldSystem.hpp>
#include <Termina/World/World.hpp>
#include <Termina/Core/Logger.hpp>

void GameManagerComponent::Start()
{
    m_GameStarted = false;
    TN_INFO("GameManagerComponent started on actor '%s'", m_Name.c_str());
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

    const auto& actors = world->GetActors();
    for (const auto& actorPtr : actors)
    {
        Termina::Actor* actor = actorPtr.get();
        if (!actor)
            continue;

        if (!actor->HasComponent<MenuComponent>())
            continue;

        MenuComponent* menu = &actor->GetComponent<MenuComponent>();
        HandleMenu(menu);
        break;
    }
}

void GameManagerComponent::HandleMenu(MenuComponent* menu)
{
    if (!menu)
        return;

    if (menu->IsPlayRequested() && !m_GameStarted)
    {
        m_GameStarted = true;
        menu->ResetRequests();

        TN_INFO("Loading game world");

        auto* worldSystem = Termina::Application::GetSystem<Termina::WorldSystem>();
        if (worldSystem)
        {
            worldSystem->LoadWorld("Assets/Worlds/Maps/map_teva1");
        }

        return;
    }

    if (menu->IsQuitRequested())
    {
        menu->ResetRequests();
        TN_INFO("Quit requested");
        exit(0);
    }
}
