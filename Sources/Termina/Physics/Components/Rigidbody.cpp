#include "Rigidbody.hpp"
#include "Collider.hpp"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/MassProperties.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/Body/AllowedDOFs.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

#include <Termina/Core/Application.hpp>
#include <Termina/Core/Logger.hpp>
#include <Termina/World/Actor.hpp>
#include <Termina/World/Components/Transform.hpp>
#include <Termina/Physics/PhysicsSystem.hpp>

#include <ImGui/imgui.h>

namespace Termina {

    static JPH::EMotionType ToJoltMotionType(Rigidbody::BodyType type)
    {
        switch (type)
        {
        case Rigidbody::BodyType::Dynamic:   return JPH::EMotionType::Dynamic;
        case Rigidbody::BodyType::Kinematic: return JPH::EMotionType::Kinematic;
        case Rigidbody::BodyType::Static:    return JPH::EMotionType::Static;
        default:                             return JPH::EMotionType::Dynamic;
        }
    }

    static JPH::EAllowedDOFs BuildDOFs(const Rigidbody& rb)
    {
        using DOF = JPH::EAllowedDOFs;
        DOF flags = DOF::All;
        if (rb.FreezePositionX) flags &= ~DOF::TranslationX;
        if (rb.FreezePositionY) flags &= ~DOF::TranslationY;
        if (rb.FreezePositionZ) flags &= ~DOF::TranslationZ;
        if (rb.FreezeRotationX) flags &= ~DOF::RotationX;
        if (rb.FreezeRotationY) flags &= ~DOF::RotationY;
        if (rb.FreezeRotationZ) flags &= ~DOF::RotationZ;
        return flags;
    }

    bool Rigidbody::IsBodyValid() const
    {
        return m_BodyID.IsInvalid() == false;
    }

    void Rigidbody::OnPlay()
    {
        if (!m_Owner) return;

        // Find the Collider sibling.
        Collider* collider = nullptr;
        for (auto* comp : m_Owner->GetAllComponents())
        {
            if (auto* c = dynamic_cast<Collider*>(comp))
            {
                collider = c;
                break;
            }
        }

        if (!collider)
        {
            TN_ERROR("Rigidbody on '%s': no Collider component found — body not created",
                     m_Owner->GetName().c_str());
            return;
        }

        JPH::ShapeRefC shape = collider->CreateShape();
        if (!shape)
        {
            TN_ERROR("Rigidbody on '%s': Collider failed to create shape", m_Owner->GetName().c_str());
            return;
        }

        auto& transform = m_Owner->GetComponent<Transform>();

        // Apply world-space scale to the physics shape so colliders match the
        // visual size of scaled actors.
        glm::vec3 s = transform.GetScale();
        if (s.x != 1.0f || s.y != 1.0f || s.z != 1.0f)
        {
            JPH::ScaledShapeSettings scaled(shape, JPH::Vec3(s.x, s.y, s.z));
            auto res = scaled.Create();
            if (res.IsValid())
                shape = res.Get();
        }

        glm::vec3 pos = transform.GetPosition();
        glm::quat rot = transform.GetRotation();

        // Pick object layer.
        JPH::ObjectLayer layer;
        if (IsSensor)
            layer = PhysicsLayers::TRIGGER;
        else if (Type == BodyType::Static)
            layer = PhysicsLayers::NON_MOVING;
        else
            layer = PhysicsLayers::MOVING;

        JPH::BodyCreationSettings bcs(
            shape,
            JPH::RVec3(pos.x, pos.y, pos.z),
            JPH::Quat(rot.x, rot.y, rot.z, rot.w),
            ToJoltMotionType(Type),
            layer
        );

        bcs.mIsSensor        = IsSensor;
        bcs.mAllowSleeping   = AllowSleep;
        bcs.mGravityFactor   = GravityFactor;
        bcs.mLinearDamping   = LinearDamping;
        bcs.mAngularDamping  = AngularDamping;
        bcs.mAllowedDOFs     = BuildDOFs(*this);

        if (Type == BodyType::Dynamic)
        {
            bcs.mMassPropertiesOverride.mMass = Mass;
            bcs.mOverrideMassProperties       = JPH::EOverrideMassProperties::CalculateInertia;
        }

        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::BodyInterface& bi = physics->GetBodyInterface();

        JPH::EActivation activation = (Type == BodyType::Static)
            ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

        m_BodyID = bi.CreateAndAddBody(bcs, activation);
        if (m_BodyID.IsInvalid())
        {
            TN_ERROR("Rigidbody on '%s': Jolt failed to create body", m_Owner->GetName().c_str());
            return;
        }

        // Store Actor* as user data so contact events can find us.
        bi.SetUserData(m_BodyID, reinterpret_cast<uint64_t>(m_Owner));
        physics->RegisterBody(m_BodyID, m_Owner);
    }

    void Rigidbody::OnStop()
    {
        if (m_BodyID.IsInvalid()) return;

        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::BodyInterface& bi = physics->GetBodyInterface();
        bi.RemoveBody(m_BodyID);
        bi.DestroyBody(m_BodyID);
        physics->UnregisterBody(m_BodyID);
        m_BodyID = JPH::BodyID();
    }

    void Rigidbody::OnPrePhysics(float dt)
    {
        if (m_BodyID.IsInvalid()) return;
        if (Type == BodyType::Dynamic) return; // dynamic bodies are driven by Jolt

        auto& transform = m_Owner->GetComponent<Transform>();
        glm::vec3 pos = transform.GetPosition();
        glm::quat rot = transform.GetRotation();

        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::BodyInterface& bi = physics->GetBodyInterface();

        if (Type == BodyType::Kinematic)
        {
            bi.MoveKinematic(m_BodyID,
                             JPH::RVec3(pos.x, pos.y, pos.z),
                             JPH::Quat(rot.x, rot.y, rot.z, rot.w),
                             dt);
        }
        else // Static — sync if transform was manually changed (editor-placed)
        {
            bi.SetPositionAndRotation(m_BodyID,
                                      JPH::RVec3(pos.x, pos.y, pos.z),
                                      JPH::Quat(rot.x, rot.y, rot.z, rot.w),
                                      JPH::EActivation::DontActivate);
        }
    }

    void Rigidbody::OnPostPhysics(float /*dt*/)
    {
        if (m_BodyID.IsInvalid()) return;
        if (Type != BodyType::Dynamic) return;

        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::BodyInterface& bi = physics->GetBodyInterface();

        JPH::RVec3 jPos;
        JPH::Quat  jRot;
        bi.GetPositionAndRotation(m_BodyID, jPos, jRot);

        auto& transform = m_Owner->GetComponent<Transform>();
        transform.SetPosition(glm::vec3(jPos.GetX(), jPos.GetY(), jPos.GetZ()));
        transform.SetRotation(glm::quat(jRot.GetW(), jRot.GetX(), jRot.GetY(), jRot.GetZ()));
    }

    void Rigidbody::AddForce(const glm::vec3& force)
    {
        if (m_BodyID.IsInvalid()) return;
        auto* physics = Application::GetSystem<PhysicsSystem>();
        physics->GetBodyInterface().AddForce(m_BodyID, JPH::Vec3(force.x, force.y, force.z));
    }

    void Rigidbody::AddImpulse(const glm::vec3& impulse)
    {
        if (m_BodyID.IsInvalid()) return;
        auto* physics = Application::GetSystem<PhysicsSystem>();
        physics->GetBodyInterface().AddImpulse(m_BodyID, JPH::Vec3(impulse.x, impulse.y, impulse.z));
    }

    void Rigidbody::AddTorque(const glm::vec3& torque)
    {
        if (m_BodyID.IsInvalid()) return;
        auto* physics = Application::GetSystem<PhysicsSystem>();
        physics->GetBodyInterface().AddTorque(m_BodyID, JPH::Vec3(torque.x, torque.y, torque.z));
    }

    void Rigidbody::SetLinearVelocity(const glm::vec3& velocity)
    {
        if (m_BodyID.IsInvalid()) return;
        auto* physics = Application::GetSystem<PhysicsSystem>();
        physics->GetBodyInterface().SetLinearVelocity(m_BodyID, JPH::Vec3(velocity.x, velocity.y, velocity.z));
    }

    glm::vec3 Rigidbody::GetLinearVelocity() const
    {
        if (m_BodyID.IsInvalid()) return glm::vec3(0.0f);
        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::Vec3 v = physics->GetBodyInterface().GetLinearVelocity(m_BodyID);
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    glm::vec3 Rigidbody::GetAngularVelocity() const
    {
        if (m_BodyID.IsInvalid()) return glm::vec3(0.0f);
        auto* physics = Application::GetSystem<PhysicsSystem>();
        JPH::Vec3 v = physics->GetBodyInterface().GetAngularVelocity(m_BodyID);
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    void Rigidbody::Inspect()
    {
        const char* types[] = { "Dynamic", "Static", "Kinematic" };
        int current = static_cast<int>(Type);
        if (ImGui::Combo("Type", &current, types, 3))
            Type = static_cast<BodyType>(current);

        ImGui::DragFloat("Mass",            &Mass,           0.01f, 0.001f, 10000.0f);
        ImGui::DragFloat("Linear Damping",  &LinearDamping,  0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Angular Damping", &AngularDamping, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Gravity Factor",  &GravityFactor,  0.01f, 0.0f, 10.0f);
        ImGui::Checkbox("Is Sensor",   &IsSensor);
        ImGui::Checkbox("Allow Sleep", &AllowSleep);

        if (ImGui::TreeNode("Freeze Position"))
        {
            ImGui::Checkbox("X##PX", &FreezePositionX);
            ImGui::SameLine();
            ImGui::Checkbox("Y##PY", &FreezePositionY);
            ImGui::SameLine();
            ImGui::Checkbox("Z##PZ", &FreezePositionZ);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Freeze Rotation"))
        {
            ImGui::Checkbox("X##RX", &FreezeRotationX);
            ImGui::SameLine();
            ImGui::Checkbox("Y##RY", &FreezeRotationY);
            ImGui::SameLine();
            ImGui::Checkbox("Z##RZ", &FreezeRotationZ);
            ImGui::TreePop();
        }
    }

    void Rigidbody::Serialize(nlohmann::json& out) const
    {
        out["type"]             = static_cast<int>(Type);
        out["mass"]             = Mass;
        out["linear_damping"]   = LinearDamping;
        out["angular_damping"]  = AngularDamping;
        out["gravity_factor"]   = GravityFactor;
        out["is_sensor"]        = IsSensor;
        out["allow_sleep"]      = AllowSleep;
        out["freeze_pos"]       = { FreezePositionX, FreezePositionY, FreezePositionZ };
        out["freeze_rot"]       = { FreezeRotationX, FreezeRotationY, FreezeRotationZ };
    }

    void Rigidbody::Deserialize(const nlohmann::json& in)
    {
        if (in.contains("type"))            Type            = static_cast<BodyType>(in["type"].get<int>());
        if (in.contains("mass"))            Mass            = in["mass"];
        if (in.contains("linear_damping"))  LinearDamping   = in["linear_damping"];
        if (in.contains("angular_damping")) AngularDamping  = in["angular_damping"];
        if (in.contains("gravity_factor"))  GravityFactor   = in["gravity_factor"];
        if (in.contains("is_sensor"))       IsSensor        = in["is_sensor"];
        if (in.contains("allow_sleep"))     AllowSleep      = in["allow_sleep"];
        if (in.contains("freeze_pos"))
        {
            FreezePositionX = in["freeze_pos"][0];
            FreezePositionY = in["freeze_pos"][1];
            FreezePositionZ = in["freeze_pos"][2];
        }
        if (in.contains("freeze_rot"))
        {
            FreezeRotationX = in["freeze_rot"][0];
            FreezeRotationY = in["freeze_rot"][1];
            FreezeRotationZ = in["freeze_rot"][2];
        }
    }

} // namespace Termina
