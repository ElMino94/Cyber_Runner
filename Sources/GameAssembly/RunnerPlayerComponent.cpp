#include "RunnerPlayerComponent.hpp"

#include <ImGui/imgui.h>
#include <Termina/Core/Logger.hpp>

#include <algorithm>

void RunnerPlayerComponent::Start()
{
    m_StartPosition = m_Transform->GetPosition();
    m_GroundY = m_StartPosition.y;

    m_CurrentLane = 0;
    m_VerticalVelocity = 0.0f;
    m_IsGrounded = true;
    m_IsDead = false;

    TN_INFO("RunnerPlayerComponent started on actor '%s'", m_Name.c_str());
}

void RunnerPlayerComponent::Update(float deltaTime)
{
    if (m_IsDead)
        return;

    HandleInput();
    MoveForward(deltaTime);
    MoveToTargetLane(deltaTime);
    UpdateJump(deltaTime);
}

void RunnerPlayerComponent::HandleInput()
{
    if (TerminaScript::Input::IsKeyPressed(Termina::Key::D) ||
        TerminaScript::Input::IsKeyPressed(Termina::Key::Left))
    {
        m_CurrentLane = (std::max)(-1, m_CurrentLane - 1);
    }

    if (TerminaScript::Input::IsKeyPressed(Termina::Key::A) ||
        TerminaScript::Input::IsKeyPressed(Termina::Key::Right))
    {
        m_CurrentLane = (std::min)(1, m_CurrentLane + 1);
    }

    if (m_IsGrounded &&
        (TerminaScript::Input::IsKeyPressed(Termina::Key::Space) ||
            TerminaScript::Input::IsKeyPressed(Termina::Key::Up) ||
            TerminaScript::Input::IsKeyPressed(Termina::Key::W)))
    {
        m_VerticalVelocity = m_JumpForce;
        m_IsGrounded = false;
    }
}

void RunnerPlayerComponent::MoveForward(float deltaTime)
{
    glm::vec3 position = m_Transform->GetPosition();

    position.z += m_ForwardSpeed * deltaTime;

    m_Transform->SetPosition(position);
}

void RunnerPlayerComponent::MoveToTargetLane(float deltaTime)
{
    glm::vec3 position = m_Transform->GetPosition();

    float targetX = m_StartPosition.x + static_cast<float>(m_CurrentLane) * m_LaneOffset;
    float deltaX = targetX - position.x;

    float step = m_LaneChangeSpeed * deltaTime;

    if (deltaX > step)
        position.x += step;
    else if (deltaX < -step)
        position.x -= step;
    else
        position.x = targetX;

    m_Transform->SetPosition(position);
}

void RunnerPlayerComponent::UpdateJump(float deltaTime)
{
    if (m_IsGrounded)
        return;

    glm::vec3 position = m_Transform->GetPosition();

    m_VerticalVelocity -= m_Gravity * deltaTime;
    position.y += m_VerticalVelocity * deltaTime;

    if (position.y <= m_GroundY)
    {
        position.y = m_GroundY;
        m_VerticalVelocity = 0.0f;
        m_IsGrounded = true;
    }

    m_Transform->SetPosition(position);
}

void RunnerPlayerComponent::OnCollisionEnter(Termina::Actor* other)
{
    if (!other)
        return;

    TN_WARN("Player collided with '%s'", other->GetName().c_str());

    // Pour le moment, toute collision = mort.
    // Plus tard on filtrera selon le nom/tag/type d'obstacle.
    KillPlayer();
}

void RunnerPlayerComponent::KillPlayer()
{
    if (m_IsDead)
        return;

    m_IsDead = true;
    TN_ERROR("Game Over");

    // Si tu veux recharger une scène plus tard :
    // LoadScene("Assets/Scenes/Main.scene");
}

void RunnerPlayerComponent::Inspect()
{
    ImGui::Text("Runner Player");

    ImGui::DragFloat("Forward Speed", &m_ForwardSpeed, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Lane Offset", &m_LaneOffset, 0.05f, 0.1f, 10.0f);
    ImGui::DragFloat("Lane Change Speed", &m_LaneChangeSpeed, 0.1f, 0.1f, 50.0f);

    ImGui::Separator();

    ImGui::DragFloat("Jump Force", &m_JumpForce, 0.1f, 0.0f, 50.0f);
    ImGui::DragFloat("Gravity", &m_Gravity, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Ground Y", &m_GroundY, 0.01f);

    ImGui::Separator();

    ImGui::Separator();

    ImGui::Text("Current Lane: %d", m_CurrentLane);
    ImGui::Text("Grounded: %s", m_IsGrounded ? "true" : "false");
    ImGui::Text("Dead: %s", m_IsDead ? "true" : "false");
    ImGui::Text("Vertical Velocity: %.2f", m_VerticalVelocity);
}

void RunnerPlayerComponent::Serialize(nlohmann::json& out) const
{
    out["forwardSpeed"] = m_ForwardSpeed;
    out["laneOffset"] = m_LaneOffset;
    out["laneChangeSpeed"] = m_LaneChangeSpeed;
    out["jumpForce"] = m_JumpForce;
    out["gravity"] = m_Gravity;
    out["groundY"] = m_GroundY;
}

void RunnerPlayerComponent::Deserialize(const nlohmann::json& in)
{
    if (in.contains("forwardSpeed"))       m_ForwardSpeed = in["forwardSpeed"];
    if (in.contains("laneOffset"))         m_LaneOffset = in["laneOffset"];
    if (in.contains("laneChangeSpeed"))    m_LaneChangeSpeed = in["laneChangeSpeed"];
    if (in.contains("jumpForce"))          m_JumpForce = in["jumpForce"];
    if (in.contains("gravity"))            m_Gravity = in["gravity"];
    if (in.contains("groundY"))            m_GroundY = in["groundY"];
}