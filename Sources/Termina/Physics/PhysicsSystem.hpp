#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include <GLM/glm.hpp>

#include <Termina/Core/System.hpp>

namespace Termina {
    constexpr float PhysicsUpdateRate = 1.0f / 90.0f;

    class Actor;

    namespace PhysicsLayers
    {
        static constexpr uint8 NON_MOVING = 0;
        static constexpr uint8 MOVING = 1;
        static constexpr uint8 CHARACTER = 2;
        static constexpr uint8 CHARACTER_GHOST = 3;
        static constexpr uint8 TRIGGER = 4;
        static constexpr uint8 NUM_LAYERS = 5;
    };

    struct Ray
    {
        glm::vec3 Origin;
        glm::vec3 Direction;
        float MaxDistance;
    };

    struct RayResult
    {
        bool Hit = false;
        glm::vec3 Position;
        float T = 0.0f;
        Actor* HitActor = nullptr;
    };

    class PhysicsSystem : public ISystem
    {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        void Physics(float deltaTime) override;

        void RegisterComponents() override {}
        void UnregisterComponents() override {}

        void SetGravity(const glm::vec3& gravity);
        RayResult Raycast(const Ray& ray) const;

        UpdateFlags GetUpdateFlags() const override { return (UpdateFlags)0; }
        std::string GetName()        const override { return "Physics System"; }
        int         GetPriority()    const override { return 10; }
    private:
        JPH::PhysicsSystem* m_System = nullptr;
        JPH::JobSystemThreadPool* m_ThreadPool = nullptr;
        JPH::BodyInterface* m_BodyInterface = nullptr;
        JPH::ContactListener* m_ContactListener = nullptr;
        JPH::TempAllocatorMalloc* m_TempAllocator = nullptr;
    };
}
