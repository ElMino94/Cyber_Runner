#include "WorldSystem.hpp"

#include <stdexcept>
#include <filesystem>

#include "ComponentRegistry.hpp"
#include "Components/Transform.hpp"
#include "Core/Application.hpp"
#include "Renderer/Renderer.hpp"

namespace Termina {

    WorldSystem::WorldSystem()
    {
        // Start with an empty default world so GetCurrentWorld() is never null.
        m_CurrentWorld = std::make_unique<World>();
        m_CurrentWorld->OnInit();
    }

    WorldSystem::~WorldSystem()
    {
    }

    // ---------------------------------------------------------------------------
    // World management
    // ---------------------------------------------------------------------------

    void WorldSystem::TransitionTo(std::unique_ptr<World> newWorld)
    {
        if (m_CurrentWorld)
        {
            if (m_IsPlaying)
            {
                m_CurrentWorld->OnStop();
                m_IsPlaying = false;
            }
            m_CurrentWorld->OnShutdown();
        }
        m_CurrentWorld = std::move(newWorld);
    }

    void WorldSystem::NewWorld(const std::string& name)
    {
        auto world = std::make_unique<World>();
        world->SetName(name);
        TransitionTo(std::move(world));
        m_CurrentWorld->OnInit();
    }

    bool WorldSystem::LoadWorld(const std::string& path)
    {
        // Destroy the old world first so its actors release their IDs before
        // LoadFromFile calls IDGenerator::Clear().
        if (m_CurrentWorld)
        {
            if (m_IsPlaying)
            {
                m_CurrentWorld->OnStop();
                m_IsPlaying = false;
            }
            m_CurrentWorld->OnShutdown();
            m_CurrentWorld.reset();
        }

        auto candidate = std::make_unique<World>();
        try
        {
            candidate->LoadFromFile(path);
        }
        catch (const std::exception&)
        {
            return false;
        }

        m_CurrentWorld = std::move(candidate);
        return true;
    }

    bool WorldSystem::SaveWorld(const std::string& path)
    {
        if (!m_CurrentWorld)
            return false;
        try
        {
            m_CurrentWorld->SaveToFile(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    void WorldSystem::Play()
    {
        if (!m_CurrentWorld || m_IsPlaying)
            return;

        // Snapshot the world state so Stop() can restore it.
        m_PrePlayPath = m_CurrentWorld->GetCurrentPath();
        m_PlaySnapshot = (std::filesystem::temp_directory_path() / "__termina_play_snapshot__.trw").string();
        SaveWorld(m_PlaySnapshot);

        m_IsPlaying = true;
        m_CurrentWorld->OnPlay();
    }

    void WorldSystem::Stop()
    {
        if (!m_CurrentWorld || !m_IsPlaying)
            return;
        m_CurrentWorld->OnStop();
        m_IsPlaying = false;

        // Restore the world to its pre-play state.
        if (!m_PlaySnapshot.empty() && std::filesystem::exists(m_PlaySnapshot))
        {
            // Destroy the play world BEFORE loading the snapshot so that
            // Actor::~Actor() releases IDs before LoadFromFile calls IDGenerator::Clear().
            m_CurrentWorld->OnShutdown();
            m_CurrentWorld.reset();

            auto restored = std::make_unique<World>();
            restored->LoadFromFile(m_PlaySnapshot);
            restored->SetCurrentPath(m_PrePlayPath);
            m_CurrentWorld = std::move(restored);
            // LoadFromFile already ran OnInit (Pass 3) — no second call needed.
            std::filesystem::remove(m_PlaySnapshot);
            m_PlaySnapshot.clear();
        }
    }

    void WorldSystem::RequestSceneTransition(const std::string& path)
    {
        if (!m_IsPlaying || path.empty())
            return;
        m_PendingScene = path;
    }

    // ---------------------------------------------------------------------------
    // ISystem lifecycle forwarded to the current world
    // ---------------------------------------------------------------------------

    void WorldSystem::PreUpdate(float deltaTime)
    {
        // Apply any deferred scene transition requested by scripts last frame.
        if (!m_PendingScene.empty())
        {
            std::string path = std::move(m_PendingScene);
            m_PendingScene.clear();

            // Destroy the current world first so actors release IDs before
            // LoadFromFile calls IDGenerator::Clear().
            if (m_CurrentWorld)
            {
                m_CurrentWorld->OnStop();
                m_CurrentWorld->OnShutdown();
                m_CurrentWorld.reset();
            }
            m_IsPlaying = false;

            auto candidate = std::make_unique<World>();
            bool loaded = false;
            try
            {
                candidate->LoadFromFile(path);
                loaded = true;
            }
            catch (const std::exception&) {}

            if (loaded)
            {
                m_CurrentWorld = std::move(candidate);
                // LoadFromFile already ran OnInit (Pass 3) — no second call needed.
                m_IsPlaying = true;
                m_CurrentWorld->OnPlay();
            }
        }

        if (m_CurrentWorld)
            m_CurrentWorld->OnPreUpdate(deltaTime);
    }

    void WorldSystem::Update(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnUpdate(deltaTime);
    }

    void WorldSystem::PostUpdate(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPostUpdate(deltaTime);
    }

    void WorldSystem::PrePhysics(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPrePhysics(deltaTime);
    }

    void WorldSystem::Physics(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPhysics(deltaTime);
    }

    void WorldSystem::PostPhysics(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPostPhysics(deltaTime);
    }

    void WorldSystem::PreRender(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPreRender(deltaTime);

        RendererSystem* renderer = Application::GetSystem<RendererSystem>();
        if (m_IsPlaying && renderer)
            renderer->SetCurrentCamera(m_CurrentWorld->GetMainCamera());
    }

    void WorldSystem::Render(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnRender(deltaTime);
    }

    void WorldSystem::PostRender(float deltaTime)
    {
        if (m_CurrentWorld)
            m_CurrentWorld->OnPostRender(deltaTime);
    }

    // ---------------------------------------------------------------------------
    // Component registration
    // ---------------------------------------------------------------------------

    void WorldSystem::RegisterComponents()
    {
        ComponentRegistry::Get().Register<Transform>("Transform");
    }

    void WorldSystem::UnregisterComponents()
    {
        ComponentRegistry::Get().Unregister<Transform>();
    }
}
