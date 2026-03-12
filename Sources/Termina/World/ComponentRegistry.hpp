#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <type_traits>
#include <mutex>

#include <Termina/World/Component.hpp>

namespace Termina {

    class Actor;

    // Simple registry that maps component types and names to factory functions.
    // Use this to:
    // - Register component types with a human-readable name
    // - Create components by type or by name (useful for serialization/deserialization)
    // - Iterate registered component types (e.g. for editor UI)
    //
    // Example:
    //   ComponentRegistry::Get().Register<Transform>("Transform");
    //   Component* c = ComponentRegistry::Get().CreateByName("Transform", actor);
    //
    class ComponentRegistry
    {
    public:
        struct Entry
        {
            std::string Name;
            std::type_index Type;
            std::function<Component*(Actor*)> Factory;
        };

        static ComponentRegistry& Get()
        {
            static ComponentRegistry instance;
            return instance;
        }

        // Register a component type T with a name.
        // Returns true on success, false if the type or name is already registered.
        template<typename T>
        bool Register(const std::string& name)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::type_index idx(typeid(T));
            if (m_ByType.count(idx) || m_NameToType.count(name)) return false;
            m_ByType.emplace(idx, Entry{name, idx, [](Actor* owner) -> Component* { return new T(owner); }});
            m_NameToType.emplace(name, idx);
            m_Order.push_back(idx);
            return true;
        }

        // Register with a custom factory (for components with special construction needs).
        template<typename T>
        bool RegisterFactory(const std::string& name, std::function<Component*(Actor*)> factory)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            if (!factory) return false;
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::type_index idx(typeid(T));
            if (m_ByType.count(idx) || m_NameToType.count(name)) return false;
            m_ByType.emplace(idx, Entry{name, idx, std::move(factory)});
            m_NameToType.emplace(name, idx);
            m_Order.push_back(idx);
            return true;
        }

        // Unregister a typed component. Safe to call before unloading a DLL.
        template<typename T>
        bool Unregister()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::type_index idx(typeid(T));
            auto it = m_ByType.find(idx);
            if (it == m_ByType.end()) return false;
            m_NameToType.erase(it->second.Name);
            m_ByType.erase(it);
            m_Order.erase(std::remove(m_Order.begin(), m_Order.end(), idx), m_Order.end());
            return true;
        }

        // Typed component create helper — returns nullptr if not registered.
        template<typename T>
        T* Create(Actor* owner) const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return static_cast<T*>(CreateByType(typeid(T), owner));
        }

        // Iterate all typed registered entries in registration order.
        // Return false from the callback to stop early.
        template<typename Fn>
        void ForEach(Fn&& fn) const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (const auto& idx : m_Order) {
                auto it = m_ByType.find(idx);
                if (it == m_ByType.end()) continue;
                if (!fn(it->second)) break;
            }
        }

        bool RegisterScripted(const std::string& name, std::function<Component*(Actor*)> factory);

        Component* CreateByType(std::type_index type, Actor* owner) const;
        Component* CreateByName(const std::string& name, Actor* owner) const;

        bool IsRegistered(std::type_index type) const;
        bool IsRegisteredByName(const std::string& name) const;
        std::string GetNameForType(std::type_index type) const;

        bool UnregisterByName(const std::string& name);

        size_t Count() const;
        void Clear();
        void Report() const;

    private:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry& operator=(const ComponentRegistry&) = delete;

        mutable std::mutex m_Mutex;
        std::unordered_map<std::type_index, Entry> m_ByType;
        std::unordered_map<std::string, std::type_index> m_NameToType;
        std::vector<std::type_index> m_Order;
        std::unordered_map<std::string, std::function<Component*(Actor*)>> m_ScriptedByName;
    };

} // namespace Termina

// ---------------------------------------------------------------------------
// DLL hot-reload component module macros
// ---------------------------------------------------------------------------
//
// Usage — in your DLL's module .cpp file:
//
//   COMPONENT_MODULE_BEGIN()
//       REGISTER_COMPONENT(TransformComponent, "TransformComponent")
//       REGISTER_COMPONENT_FACTORY(PhysicsComponent, "PhysicsComponent",
//           [](Termina::Actor* o) { return new PhysicsComponent(o, PhysicsWorld::Get()); })
//       REGISTER_SCRIPTED_COMPONENT("LuaHealthComponent",
//           [](Termina::Actor* o) { return new LuaComponent(o, "health"); })
//   COMPONENT_MODULE_END()
//
// The host calls RegisterComponents() after loading the DLL and
// UnregisterComponents() before unloading it, ensuring no dangling
// factory pointers remain in the registry.
//
//   void* lib = dlopen("game.dll", RTLD_NOW);
//   ((void(*)())dlsym(lib, "RegisterComponents"))();
//
//   // Hot-reload:
//   ((void(*)())dlsym(lib, "UnregisterComponents"))();
//   dlclose(lib);
//   lib = dlopen("game.dll", RTLD_NOW);
//   ((void(*)())dlsym(lib, "RegisterComponents"))();
//

#if defined(_WIN32)
#  define TERMINA_DLL_EXPORT extern "C" __declspec(dllexport)
#else
#  define TERMINA_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// _termina_module_names_ is 'static' (file-local), so each DLL .cpp gets its own
// independent list — hot-reloading multiple DLLs simultaneously is safe.
#define COMPONENT_MODULE_BEGIN()                                                        \
    static std::vector<std::string> _termina_module_names_;                            \
    TERMINA_DLL_EXPORT void UnregisterComponents() {                                    \
        for (const auto& _n_ : _termina_module_names_)                                 \
            ::Termina::ComponentRegistry::Get().UnregisterByName(_n_);                 \
        _termina_module_names_.clear();                                                 \
    }                                                                                   \
    TERMINA_DLL_EXPORT void RegisterComponents() {

#define COMPONENT_MODULE_END()                                                          \
    }                                                                                   \
    TERMINA_DLL_EXPORT const std::vector<std::string>& GetComponentNames() {            \
        return _termina_module_names_;                                                  \
    }

#define REGISTER_COMPONENT(Type, Name)                                      \
    ::Termina::ComponentRegistry::Get().Register<Type>(Name);               \
    _termina_module_names_.push_back(Name);

#define REGISTER_COMPONENT_FACTORY(Type, Name, Factory)                     \
    ::Termina::ComponentRegistry::Get().RegisterFactory<Type>(Name, Factory); \
    _termina_module_names_.push_back(Name);

#define REGISTER_SCRIPTED_COMPONENT(Name, Factory)                          \
    ::Termina::ComponentRegistry::Get().RegisterScripted(Name, Factory);    \
    _termina_module_names_.push_back(Name);
