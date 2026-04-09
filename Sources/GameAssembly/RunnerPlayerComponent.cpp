#include "RunnerPlayerComponent.hpp"
#include "CollectibleComponent.hpp"
#include "JumpBoostPickup.hpp"

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

    m_JumpForce = m_BaseJumpForce;
    m_JumpBoostTimer = 0.0f;
    m_HasJumpBoost = false;

    TN_INFO("RunnerPlayerComponent started on actor '%s'", m_Name.c_str());
}

void RunnerPlayerComponent::Update(float deltaTime)
{
    if (m_IsDead)
        return;

    UpdateJumpBoost(deltaTime);

    HandleInput();
    MoveForward(deltaTime);
    MoveToTargetLane(deltaTime);
    UpdateJump(deltaTime);
}

void RunnerPlayerComponent::HandleInput()
{
    // Gauche
    if (TerminaScript::Input::IsKeyPressed(Termina::Key::D) ||
        TerminaScript::Input::IsKeyPressed(Termina::Key::Left))
    {
        m_CurrentLane = (std::max)(-1, m_CurrentLane - 1);
    }

    // Droite
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

void RunnerPlayerComponent::UpdateJumpBoost(float deltaTime)
{
    if (!m_HasJumpBoost)
        return;

    m_JumpBoostTimer -= deltaTime;

    if (m_JumpBoostTimer <= 0.0f)
    {
        m_JumpBoostTimer = 0.0f;
        m_HasJumpBoost = false;
        m_JumpForce = m_BaseJumpForce;

        TN_INFO("Jump boost expired | Back to base jump force: %.2f", m_BaseJumpForce);
    }
}

void RunnerPlayerComponent::ActivateJumpBoost(float multiplier, float duration)
{
    m_HasJumpBoost = true;
    m_JumpBoostMultiplier = multiplier;
    m_JumpBoostDuration = duration;
    m_JumpBoostTimer = duration;
    m_JumpForce = m_BaseJumpForce + 5.0f;

    TN_INFO("Jump boost activated! BaseForce: %.2f | Multiplier: %.2f | NewForce: %.2f | Duration: %.2f", 
            m_BaseJumpForce, multiplier, m_JumpForce, duration);
}

void RunnerPlayerComponent::OnCollisionEnter(Termina::Actor* other)
{
    if (!other)
        return;

    TN_INFO("Collision detected with %s", other->GetName().c_str());

    if (other->HasComponent<Collectibles>())
    {
        auto& collectible = other->GetComponent<Collectibles>();

        if (!collectible.IsCollected())
        {
            collectible.Collect();
            m_Score += collectible.value;
            Destroy(other);
        }

        return;
    }

    if (other->HasComponent<JumpBoostPickup>())
    {
        auto& jumpBoost = other->GetComponent<JumpBoostPickup>();

        ActivateJumpBoost(jumpBoost.multiplier, jumpBoost.duration);
        Destroy(other);
        return;
    }

    KillPlayer();
}

void RunnerPlayerComponent::KillPlayer()
{
    if (m_IsDead)
        return;

    m_IsDead = true;
    TN_ERROR("Game Over!");
    
    // Charger la map Game Over
    std::string gameOverPath = std::string("Assets/Worlds/Maps/map_GameOver");
    LoadScene(gameOverPath);
}

void RunnerPlayerComponent::Inspect()
{
    ImGui::Text("Runner Player");

    ImGui::DragFloat("Forward Speed", &m_ForwardSpeed, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Lane Offset", &m_LaneOffset, 0.05f, 0.1f, 10.0f);
    ImGui::DragFloat("Lane Change Speed", &m_LaneChangeSpeed, 0.1f, 0.1f, 50.0f);

    ImGui::Separator();

    ImGui::DragFloat("Base Jump Force", &m_BaseJumpForce, 0.1f, 0.0f, 50.0f);
    ImGui::Text("Current Jump Force: %.2f", m_JumpForce);
    ImGui::DragFloat("Gravity", &m_Gravity, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Ground Y", &m_GroundY, 0.01f);

    ImGui::Separator();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "=== Jump Boost Status ===");
    ImGui::Text("Boost Active: %s", m_HasJumpBoost ? "YES" : "NO");
    if (m_HasJumpBoost)
    {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Active Multiplier: %.2f", m_JumpBoostMultiplier);
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Time Remaining: %.2f / %.2f sec", m_JumpBoostTimer, m_JumpBoostDuration);
    }
    ImGui::DragFloat("Default Boost Multiplier", &m_JumpBoostMultiplier, 0.05f, 1.0f, 5.0f);
    ImGui::DragFloat("Default Boost Duration", &m_JumpBoostDuration, 0.1f, 0.0f, 30.0f);

    ImGui::Separator();

    ImGui::Text("Current Lane: %d", m_CurrentLane);
    ImGui::Text("Grounded: %s", m_IsGrounded ? "true" : "false");
    ImGui::Text("Dead: %s", m_IsDead ? "true" : "false");
    ImGui::Text("Vertical Velocity: %.2f", m_VerticalVelocity);
    ImGui::Text("Score: %d", m_Score);
}

void RunnerPlayerComponent::Serialize(nlohmann::json& out) const
{
    out["forwardSpeed"] = m_ForwardSpeed;
    out["laneOffset"] = m_LaneOffset;
    out["laneChangeSpeed"] = m_LaneChangeSpeed;

    out["baseJumpForce"] = m_BaseJumpForce;
    out["gravity"] = m_Gravity;
    out["groundY"] = m_GroundY;

    out["jumpBoostMultiplier"] = m_JumpBoostMultiplier;
    out["jumpBoostDuration"] = m_JumpBoostDuration;
}

void RunnerPlayerComponent::Deserialize(const nlohmann::json& in)
{
    if (in.contains("forwardSpeed"))         m_ForwardSpeed = in["forwardSpeed"];
    if (in.contains("laneOffset"))           m_LaneOffset = in["laneOffset"];
    if (in.contains("laneChangeSpeed"))      m_LaneChangeSpeed = in["laneChangeSpeed"];

    if (in.contains("baseJumpForce"))        m_BaseJumpForce = in["baseJumpForce"];
    if (in.contains("gravity"))              m_Gravity = in["gravity"];
    if (in.contains("groundY"))              m_GroundY = in["groundY"];

    if (in.contains("jumpBoostMultiplier"))  m_JumpBoostMultiplier = in["jumpBoostMultiplier"];
    if (in.contains("jumpBoostDuration"))    m_JumpBoostDuration = in["jumpBoostDuration"];

    m_JumpForce = m_BaseJumpForce;
}