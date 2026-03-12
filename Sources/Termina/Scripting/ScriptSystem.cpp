#include "ScriptSystem.hpp"
#include "Core/Application.hpp"
#include "Core/FileSystem.hpp"
#include "Platform/LaunchProcess.hpp"
#include "Scripting/ScriptModuleManager.hpp"
#include "World/WorldSystem.hpp"
#include "Core/Logger.hpp"

namespace Termina {
    ScriptSystem::ScriptSystem()
    {
#if defined(TRMN_LINUX)
        std::string libName = "libGameAssembly.so";
#elif defined(TRMN_WINDOWS)
        std::string libName = "libGameAssembly.dylib";
#elif defined(TRMN_MACOS)
        std::string libName = "GameAssembly.dll";
#endif

        ScriptModuleManager::Get().Load("Game", libName);
        RebuildWatches();
    }

    ScriptSystem::~ScriptSystem()
    {
        for (auto& [name, path] : ScriptModuleManager::Get().GetLoadedModules())
            ScriptModuleManager::Get().Unload(name);
    }

    bool ScriptSystem::Compile()
    {
        return LaunchProcess::Launch("xmake", {"build", "GameAssembly"}) == 0;
    }

    bool ScriptSystem::Recompile()
    {
        LaunchProcess::Launch("xmake", {"clean", "GameAssembly"});
        return LaunchProcess::Launch("xmake", {"build", "GameAssembly"}) == 0;
    }

    void ScriptSystem::RebuildWatches()
    {
        m_Watches.clear();
        for (auto& file : FileSystem::GetFilesRecursive("Sources/GameAssembly")) {
            if (FileSystem::HasExtension(file, ".cpp") || FileSystem::HasExtension(file, ".hpp"))
                m_Watches.push_back(FileSystem::WatchFile(file));
        }
    }

    void ScriptSystem::Update(float deltaTime)
    {
        // Execute reload at start of frame (priority -1 ensures we run before WorldSystem)
        // so no component list is being iterated when we add/remove components.
        if (m_PendingReload) {
            m_PendingReload = false;
            if (!Compile()) {
                TN_ERROR("Failed to compile scripts");
                RebuildWatches();
                return;
            } else {
                World* world = Application::GetSystem<WorldSystem>()->GetCurrentWorld();
                ScriptModuleManager::Get().Reload("Game", world);
                RebuildWatches();
                return;
            }
        }

        for (const auto& watch : m_Watches) {
            if (FileSystem::HasFileChanged(watch)) {
                m_PendingReload = true;
                return;
            }
        }
    }
}
