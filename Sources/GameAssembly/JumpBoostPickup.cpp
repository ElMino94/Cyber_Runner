#include "JumpBoostPickup.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Logger.hpp>
#include <Termina/Physics/Components/SphereCollider.hpp>
#include <Termina/Physics/Components/Rigidbody.hpp>
#include <Termina/Renderer/Components/MeshComponent.hpp>
#include "RunnerPlayerComponent.hpp"

void JumpBoostPickup::Start()
{
    TN_INFO("JumpBoostPickup started on actor '%s' | Multiplier: %.2f | Duration: %.2f", 
            m_Name.c_str(), multiplier, duration);
    
    // Récupérer le joueur
    m_Player = m_Owner->GetParentWorld()->GetActorByName("Player");
    
    if (m_Player)
    {
        TN_INFO("JumpBoostPickup: Joueur trouvé");
    }
    else
    {
        TN_WARN("JumpBoostPickup: Impossible de trouver le joueur!");
    }
}

void JumpBoostPickup::Update(float deltaTime)
{
    glm::quat rotation = m_Transform->GetRotation();

    glm::quat delta = glm::angleAxis(glm::radians(m_RotationSpeed * deltaTime), glm::vec3(0, 1, 0));

    rotation = delta * rotation;

    m_Transform->SetRotation(rotation);
}

void JumpBoostPickup::Inspect()
{
    ImGui::Text("Jump Boost Pickup");

    ImGui::DragFloat("Multiplier", &multiplier, 0.05f, 1.0f, 5.0f);
    ImGui::DragFloat("Duration", &duration, 0.1f, 0.1f, 30.0f);
    ImGui::DragFloat("Rotation Speed", &m_RotationSpeed, 1.0f, 0.0f, 1000.0f);
}

void JumpBoostPickup::Serialize(nlohmann::json& out) const
{
    out["multiplier"] = multiplier;
    out["duration"] = duration;
    out["rotationSpeed"] = m_RotationSpeed;
}

void JumpBoostPickup::Deserialize(const nlohmann::json& in)
{
    if (in.contains("multiplier"))    multiplier = in["multiplier"];
    if (in.contains("duration"))      duration = in["duration"];
    if (in.contains("rotationSpeed")) m_RotationSpeed = in["rotationSpeed"];
}

void JumpBoostPickup::OnTriggerEnter(Termina::Actor* other)
{
    if (!other || other != m_Player)
        return;

    if (IsTaken)
        return;

    TN_INFO("JumpBoostPickup: Collision avec le joueur! Activation boost: x%.2f pour %.1f sec", 
            multiplier, duration);

    if (m_Player && m_Player->HasComponent<RunnerPlayerComponent>())
    {
        auto& playerComponent = m_Player->GetComponent<RunnerPlayerComponent>();
        playerComponent.ActivateJumpBoost(multiplier, duration);
    }
    else
    {
        TN_ERROR("JumpBoostPickup: Le joueur n'a pas le composant RunnerPlayerComponent!");
    }

    IsTaken = true;

    // Supprimer le composant mesh pour masquer l'objet
    if (m_Owner->HasComponent<Termina::MeshComponent>())
        m_Owner->RemoveComponent<Termina::MeshComponent>();

    // Désactiver tous les composants restants
    for (auto* comp : m_Owner->GetAllComponents())
        comp->SetActive(false);

    // Désactiver l'actor lui-même
    m_Owner->SetActive(false);
}