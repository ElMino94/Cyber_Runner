#include "CameraComponent.hpp"
#include "GLM/ext/matrix_clip_space.hpp"
#include "GLM/ext/matrix_transform.hpp"
#include "ImGui/imgui.h"
#include "World/Components/Transform.hpp"

#include <Termina/World/Actor.hpp>
#include <Termina/World/World.hpp>

namespace Termina {
    CameraComponent::~CameraComponent()
    {
        m_Owner->GetParentWorld()->SetMainCamera(nullptr);
    }

    void CameraComponent::OnUpdate(float deltaTime)
    {
        Transform& transform = GetOwner()->GetComponent<Transform>();

        m_Camera.Position = transform.GetPosition();
        m_Camera.Direction = transform.GetForward();

        m_Camera.Projection = glm::perspective(glm::radians(m_FOV), 16.0f / 9.0f, m_Camera.Near, m_Camera.Far);
        m_Camera.Projection[1][1] *= -1;
        m_Camera.View = glm::inverse(transform.GetWorldMatrix());
        m_Camera.ViewProjection = m_Camera.Projection * m_Camera.View;
    }

    void CameraComponent::OnPreUpdate(float deltaTime)
    {
        if (m_Primary)
            m_Owner->GetParentWorld()->SetMainCamera(m_Owner);
    }

    void CameraComponent::OnPostRender(float deltaTime)
    {
        m_Camera.UpdatePrevious();
    }

    void CameraComponent::Inspect()
    {
        ImGui::Checkbox("Primary", &m_Primary);
        ImGui::DragFloat("Near", &m_Camera.Near, 0.01f, 0.01f, 100.0f);
        ImGui::DragFloat("Far", &m_Camera.Far, 0.01f, 0.01f, 100.0f);
        ImGui::DragFloat("FOV", &m_FOV, 1.0f, 1.0f, 179.0f);
    }
}
