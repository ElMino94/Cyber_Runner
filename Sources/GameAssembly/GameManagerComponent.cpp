#include "GameManagerComponent.hpp"
#include "MenuComponent.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/World/WorldSystem.hpp>
#include <Termina/World/World.hpp>

void GameManagerComponent::Update(float deltaTime)
{
    Termina::World* world =
        Termina::Application::GetSystem<Termina::WorldSystem>()->GetCurrentWorld();

    for (auto& actorPtr : world->GetActors())
    {
        Termina::Actor* actor = actorPtr.get();

        for (auto* comp : actor->GetAllComponents())
        {
            MenuComponent* menu = dynamic_cast<MenuComponent*>(comp);
            if (!menu)
                continue;

            if (menu->IsPlayRequested())
            {
                // start game
            }

            if (menu->IsQuitRequested())
            {
               // Quit Game 
            }
        }
    }
}