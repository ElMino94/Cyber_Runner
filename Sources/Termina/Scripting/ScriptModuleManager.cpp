#include "ScriptModuleManager.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/Core/Logger.hpp>
#include <Termina/Renderer/Renderer.hpp>
#include <Termina/World/ComponentRegistry.hpp>
#include <Termina/World/World.hpp>

#include <JSON/json.hpp>

namespace Termina {
    ScriptModuleManager& ScriptModuleManager::Get()
    {
        static ScriptModuleManager instance;
        return instance;
    }

    static void PropagateImGuiContext(DLL* dll)
    {
        void* sym = dll->GetSymbol("SetImGuiContext");
        if (!sym) return;
        auto* renderer = Application::GetSystem<RendererSystem>();
        if (!renderer) return;
        void* ctx = renderer->GetImGuiContext();
        void* af = nullptr, *ff = nullptr, *ud = nullptr;
        renderer->GetImGuiAllocator(&af, &ff, &ud);
        reinterpret_cast<void(*)(void*, void*, void*, void*)>(sym)(ctx, af, ff, ud);
    }

    bool ScriptModuleManager::Load(const std::string& name, const std::string& path)
    {
        if (m_Modules.count(name)) {
            TN_ERROR("Module '%s' is already loaded.", name.c_str());
            return false;
        }

        auto dll = std::make_unique<DLL>(path.c_str());
        if (!dll->IsValid()) {
            TN_WARN("Module '%s': DLL '%s' not found, scripting disabled.", name.c_str(), path.c_str());
            return false;
        }
        void* regSym = dll->GetSymbol("RegisterComponents");
        if (!regSym) {
            TN_ERROR("'%s' has no RegisterComponents symbol.", path.c_str());
            return false;
        }

        reinterpret_cast<void(*)()>(regSym)();
        PropagateImGuiContext(dll.get());

        std::vector<std::string> names;
        void* namesSym = dll->GetSymbol("GetComponentNames");
        if (namesSym) {
            using GetNamesFn = const std::vector<std::string>&(*)();
            names = reinterpret_cast<GetNamesFn>(namesSym)();
        }

        m_Modules.emplace(name, LoadedModule{ name, path, std::move(dll), std::move(names) });
        return true;
    }

    void ScriptModuleManager::Reload(const std::string& name, World* world)
    {
        auto it = m_Modules.find(name);
        if (it == m_Modules.end()) {
            TN_ERROR("Cannot reload '%s': not loaded.", name.c_str());
            return;
        }

        LoadedModule& mod = it->second;
        const std::string path = mod.path;
        const std::vector<std::string> componentNames = mod.componentNames;

        // Step 1: Serialize affected component state while the old DLL is still live.
        struct SavedComponent {
            Actor* actor;
            std::string type;
            bool active;
            nlohmann::json data;
        };
        std::vector<SavedComponent> saved;

        if (world) {
            for (auto& actorPtr : world->GetActors()) {
                Actor* actor = actorPtr.get();
                for (Component* comp : actor->GetAllComponents()) {
                    std::string typeName = ComponentRegistry::Get().GetNameForType(typeid(*comp));
                    bool isModuleComp = false;
                    for (const auto& n : componentNames) {
                        if (n == typeName) { isModuleComp = true; break; }
                    }
                    if (!isModuleComp) continue;

                    SavedComponent sc;
                    sc.actor  = actor;
                    sc.type   = typeName;
                    sc.active = comp->IsActive();
                    comp->Serialize(sc.data);
                    saved.push_back(std::move(sc));
                }
            }

            // Step 2a: Remove old instances while old DLL is open so vtables are still valid.
            for (const auto& sc : saved) {
                for (Component* comp : sc.actor->GetAllComponents()) {
                    if (ComponentRegistry::Get().GetNameForType(typeid(*comp)) == sc.type) {
                        sc.actor->RemoveComponentRaw(comp);
                        break;
                    }
                }
            }
        }

        // Step 2b: Unregister factories and unload old DLL.
        void* unregSym = mod.dll->GetSymbol("UnregisterComponents");
        if (unregSym)
            reinterpret_cast<void(*)()>(unregSym)();
        mod.dll.reset();

        // Step 3: Load new DLL and register components.
        auto newDll = std::make_unique<DLL>(path.c_str());
        void* regSym = newDll->GetSymbol("RegisterComponents");
        if (!regSym) {
            TN_ERROR("Reloaded '%s' has no RegisterComponents symbol.", path.c_str());
            return;
        }
        reinterpret_cast<void(*)()>(regSym)();
        PropagateImGuiContext(newDll.get());

        std::vector<std::string> newNames;
        void* namesSym = newDll->GetSymbol("GetComponentNames");
        if (namesSym) {
            using GetNamesFn = const std::vector<std::string>&(*)();
            newNames = reinterpret_cast<GetNamesFn>(namesSym)();
        }

        mod.dll = std::move(newDll);
        mod.componentNames = newNames;

        // Step 4: Re-create and restore component state.
        if (world) {
            for (const auto& sc : saved) {
                Component* fresh = ComponentRegistry::Get().CreateByName(sc.type, sc.actor);
                if (!fresh) {
                    TN_ERROR("Could not re-create '%s' after reload.", sc.type.c_str());
                    continue;
                }
                fresh->SetActive(sc.active);
                fresh->Deserialize(sc.data);
                sc.actor->AddComponentRaw(fresh);
            }
        }
    }

    void ScriptModuleManager::Unload(const std::string& name)
    {
        auto it = m_Modules.find(name);
        if (it == m_Modules.end()) return;

        void* unregSym = it->second.dll->GetSymbol("UnregisterComponents");
        if (unregSym)
            reinterpret_cast<void(*)()>(unregSym)();

        m_Modules.erase(it);
    }

    bool ScriptModuleManager::IsLoaded(const std::string& name) const
    {
        return m_Modules.count(name) > 0;
    }

    std::vector<std::pair<std::string, std::string>> ScriptModuleManager::GetLoadedModules() const
    {
        std::vector<std::pair<std::string, std::string>> result;
        result.reserve(m_Modules.size());
        for (const auto& kv : m_Modules)
            result.push_back({ kv.second.name, kv.second.path });
        return result;
    }

    std::vector<std::string> ScriptModuleManager::GetComponentNamesForModule(const std::string& name) const
    {
        auto it = m_Modules.find(name);
        if (it == m_Modules.end()) return {};
        return it->second.componentNames;
    }
}
