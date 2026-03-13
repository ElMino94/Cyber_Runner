#include "PhysicsSystem.hpp"

#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/Collision/ContactListener.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Termina/Core/Logger.hpp"

namespace Termina {
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32 NUM_LAYERS(2);
    };

    std::string LayerToString(uint8 layer)
    {
        switch (layer)
        {
            case PhysicsLayers::NON_MOVING:
                return "NON_MOVING";
            case PhysicsLayers::MOVING:
                return "MOVING";
            case PhysicsLayers::CHARACTER:
                return "CHARACTER";
            case PhysicsLayers::CHARACTER_GHOST:
                return "CHARACTER_GHOST";
            case PhysicsLayers::TRIGGER:
                return "TRIGGER";
        }
        return "TERMINA IS COMING";
    }

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[PhysicsLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[PhysicsLayers::MOVING] = BroadPhaseLayers::MOVING;
            mObjectToBroadPhase[PhysicsLayers::CHARACTER] = BroadPhaseLayers::MOVING;
            mObjectToBroadPhase[PhysicsLayers::CHARACTER_GHOST] = BroadPhaseLayers::MOVING;
            mObjectToBroadPhase[PhysicsLayers::TRIGGER] = BroadPhaseLayers::MOVING;
        }

        virtual JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            using namespace JPH;
            JPH_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }
    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[PhysicsLayers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case PhysicsLayers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case PhysicsLayers::MOVING:
                return true;
            case PhysicsLayers::CHARACTER:
                return true;
            case PhysicsLayers::CHARACTER_GHOST:
                return true;
            case PhysicsLayers::TRIGGER:
                return inLayer2 == BroadPhaseLayers::MOVING;
            default:
                return false;
            }
        }
    };

    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case PhysicsLayers::TRIGGER:
                return inObject2 == PhysicsLayers::CHARACTER_GHOST || inObject2 == PhysicsLayers::CHARACTER || inObject2 == PhysicsLayers::MOVING;
            case PhysicsLayers::NON_MOVING:
                return inObject2 == PhysicsLayers::MOVING || inObject2 == PhysicsLayers::CHARACTER_GHOST;
            case PhysicsLayers::MOVING:
                return inObject2 == PhysicsLayers::NON_MOVING || inObject2 == PhysicsLayers::MOVING || inObject2 == PhysicsLayers::TRIGGER;
            case PhysicsLayers::CHARACTER_GHOST:
                return inObject2 == PhysicsLayers::TRIGGER; // Ghost body itself doesn't physically collide, only used for triggers
            case PhysicsLayers::CHARACTER:
                return inObject2 != PhysicsLayers::CHARACTER_GHOST && inObject2 != PhysicsLayers::CHARACTER;
            default:
                return false;
            }
        }
    };

    // TODO: Contact listener for OnTrigger/OnCollision functions

    BPLayerInterfaceImpl JoltBroadphaseLayerInterface = BPLayerInterfaceImpl();
    ObjectVsBroadPhaseLayerFilterImpl JoltObjectVSBroadphaseLayerFilter = ObjectVsBroadPhaseLayerFilterImpl();
    ObjectLayerPairFilterImpl JoltObjectVSObjectLayerFilter;

    PhysicsSystem::PhysicsSystem()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();

        JPH::RegisterTypes();

        const uint32 maxBodies = 4096;
        const uint32 numBodyMutexes = 0;
        const uint32 maxBodyPairs = 2048;
        const uint32 maxContactConstraints = 2048;
        const uint32 availableThreads = std::thread::hardware_concurrency() - 1;

        m_System = new JPH::PhysicsSystem();
        m_System->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, JoltBroadphaseLayerInterface, JoltObjectVSBroadphaseLayerFilter, JoltObjectVSObjectLayerFilter);
        m_System->SetGravity(JPH::Vec3(0.0f, -5.0f, 0.0f));

        m_BodyInterface = &m_System->GetBodyInterface();
        m_ThreadPool = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, availableThreads);
        m_TempAllocator = new JPH::TempAllocatorMalloc();
    }

    PhysicsSystem::~PhysicsSystem()
    {
        delete m_TempAllocator;
        delete m_ThreadPool;
        delete m_System;

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
    }

    void PhysicsSystem::Physics(float deltaTime)
    {
        int collisionSteps = 1;
        float minStepDuration = PhysicsUpdateRate;

        try {
            auto error = m_System->Update(minStepDuration, collisionSteps, m_TempAllocator, m_ThreadPool);

            if (error != JPH::EPhysicsUpdateError::None) {
                const char* err_msg = "";
                switch (error) {
                    case JPH::EPhysicsUpdateError::ManifoldCacheFull:
                        err_msg = "Manifold cache full";
                        break;
                    case JPH::EPhysicsUpdateError::BodyPairCacheFull:
                        err_msg = "Body pair cache full";
                        break;
                    case JPH::EPhysicsUpdateError::ContactConstraintsFull:
                        err_msg = "contact constraints full";
                        break;
                    default:
                        break;
                }
                TN_ERROR("Jolt error: %s", err_msg);
            }
        } catch (const std::exception& e) {
            TN_ERROR("Jolt exception during update: %s", e.what());
        }
    }

    void PhysicsSystem::SetGravity(const glm::vec3& gravity)
    {
        m_System->SetGravity(JPH::Vec3(gravity.x, gravity.y, gravity.z));
    }
}
