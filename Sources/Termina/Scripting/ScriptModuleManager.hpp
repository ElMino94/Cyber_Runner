#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <Termina/Platform/DLL.hpp>

namespace Termina {
    class World;

    // Manages the lifecycle of script component DLLs.
    // Call Load() before World::LoadFromFile() to ensure scripted components
    // are registered before deserialization. Call Reload() to hot-reload a
    // module while preserving component state via Serialize/Deserialize.
    class ScriptModuleManager
    {
    public:
        static ScriptModuleManager& Get()
        {
            static ScriptModuleManager instance;
            return instance;
        }

        // Load a DLL and call its RegisterComponents() entry point.
        // Returns false if already loaded or the DLL fails to open.
        bool Load(const std::string& name, const std::string& path);

        // Hot-reload a module: serialize affected components, unload the old
        // DLL, load the new one, and restore component state.
        // world may be nullptr if no live world exists.
        void Reload(const std::string& name, World* world = nullptr);

        // Unload a module and call its UnregisterComponents() entry point.
        void Unload(const std::string& name);

        bool IsLoaded(const std::string& name) const;

        // Returns {name, path} pairs for all loaded modules (used during SaveToFile).
        std::vector<std::pair<std::string, std::string>> GetLoadedModules() const;

        // Returns the component names registered by a specific module.
        std::vector<std::string> GetComponentNamesForModule(const std::string& name) const;

    private:
        ScriptModuleManager() = default;
        ~ScriptModuleManager() = default;
        ScriptModuleManager(const ScriptModuleManager&) = delete;
        ScriptModuleManager& operator=(const ScriptModuleManager&) = delete;

        struct LoadedModule
        {
            std::string name;
            std::string path;
            std::unique_ptr<DLL> dll;
            std::vector<std::string> componentNames;
        };

        std::unordered_map<std::string, LoadedModule> m_Modules;
    };
}
