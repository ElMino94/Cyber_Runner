#pragma once

#include <Termina/Core/System.hpp>
#include <ThirdParty/JSON/json.hpp>

namespace Termina {
    class Actor;

    class Component
    {
    public:
        Component() = default;
        Component(Actor* owner) : m_Owner(owner), m_Active(true) {}
        virtual ~Component() = default;

        virtual void OnInit() {}
        virtual void OnShutdown() {}
        virtual void OnPlay() {}
        virtual void OnStop() {}

        virtual void OnPreUpdate(float deltaTime) {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnPostUpdate(float deltaTime) {}
        virtual void OnPrePhysics(float deltaTime) {}
        virtual void OnPhysics(float deltaTime) {}
        virtual void OnPostPhysics(float deltaTime) {}
        virtual void OnPreRender(float deltaTime) {}
        virtual void OnRender(float deltaTime) {}
        virtual void OnPostRender(float deltaTime) {}

        virtual void OnAttach(Actor* newParent) {}
        virtual void OnDetach(Actor* oldParent) {}
        virtual void Inspect() {}

        virtual void Serialize(nlohmann::json& out) const {}
        virtual void Deserialize(const nlohmann::json& in) {}

        void SetOwner(Actor* actor) { m_Owner = actor; }
        Actor* GetOwner() { return m_Owner; }

        bool IsActive() const { return m_Active; }
        void SetActive(bool active) { m_Active = active; }

        virtual UpdateFlags GetUpdateFlags() const = 0;
    protected:
        Actor* m_Owner = nullptr;

        bool m_Active = true;
    };
}
