#include "JumpBoostPickup.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Logger.hpp>

void JumpBoostPickup::Start()
{
    TN_INFO("JumpBoostPickup started on actor '%s'", m_Name.c_str());
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