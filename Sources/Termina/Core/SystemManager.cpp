#include "SystemManager.hpp"

#include <ImGui/imgui.h>
#include <algorithm>

namespace Termina {
    SystemManager::~SystemManager()
    {
        Clean();
    }

    void SystemManager::Clean()
    {
        for (auto [_, system] : m_Subsystems) {
            system->UnregisterComponents();
        }
        for (auto [_, system] : m_Subsystems) {
            delete system;
        }
        m_Subsystems.clear();
    }

    void SystemManager::Begin()
    {
        // Create list of systems sorted on priority
        m_UpdateList.clear();
        for (auto [_, system] : m_Subsystems) {
            m_UpdateList.push_back(system);
        }
        std::sort(m_UpdateList.begin(), m_UpdateList.end(), [](const auto& a, const auto& b) {
            return a->GetPriority() < b->GetPriority();
        });
    }

    void SystemManager::PreUpdate(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::UpdateDuringEditor)) system->PreUpdate(deltaTime);
            else if (!m_IsInEditor) system->PreUpdate(deltaTime);
        }
    }

    void SystemManager::Update(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::UpdateDuringEditor)) system->Update(deltaTime);
            else if (!m_IsInEditor) system->Update(deltaTime);
        }
    }

    void SystemManager::PostUpdate(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::UpdateDuringEditor)) system->PostUpdate(deltaTime);
            else if (!m_IsInEditor) system->PostUpdate(deltaTime);
        }
    }

    void SystemManager::PrePhysics(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::PhysicsUpdateDuringEditor)) system->PrePhysics(deltaTime);
            else if (!m_IsInEditor) system->PrePhysics(deltaTime);
        }
    }

    void SystemManager::Physics(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::PhysicsUpdateDuringEditor)) system->Physics(deltaTime);
            else if (!m_IsInEditor) system->Physics(deltaTime);
        }
    }

    void SystemManager::PostPhysics(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::PhysicsUpdateDuringEditor)) system->PostPhysics(deltaTime);
            else if (!m_IsInEditor) system->PostPhysics(deltaTime);
        }
    }

    void SystemManager::PreRender(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::RenderUpdateDuringEditor)) system->PreRender(deltaTime);
            else if (!m_IsInEditor) system->PreRender(deltaTime);
        }
    }

    void SystemManager::Render(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::RenderUpdateDuringEditor)) system->Render(deltaTime);
            else if (!m_IsInEditor) system->Render(deltaTime);
        }
    }

    void SystemManager::PostRender(float deltaTime)
    {
        for (ISystem* system : m_UpdateList) {
            if (m_IsInEditor && Any(system->GetUpdateFlags(), UpdateFlags::RenderUpdateDuringEditor)) system->PostRender(deltaTime);
            else if (!m_IsInEditor) system->PostRender(deltaTime);
        }
    }

    void SystemManager::SetIsInEditor(bool isInEditor)
    {
        m_IsInEditor = isInEditor;
    }

    void SystemManager::ShowDebugWindow(bool* open)
    {
        if (!ImGui::Begin("System Manager", open))
        {
            ImGui::End();
            return;
        }

        ImGui::Text("Systems: %zu", m_UpdateList.size());
        ImGui::Separator();

        if (ImGui::BeginTable("systems", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableSetupColumn("Name",     ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Flags",    ImGuiTableColumnFlags_WidthFixed, 160.f);
            ImGui::TableHeadersRow();

            for (ISystem* system : m_UpdateList)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", system->GetPriority());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(system->GetName().c_str());

                ImGui::TableSetColumnIndex(2);
                UpdateFlags flags = system->GetUpdateFlags();
                char buf[64] = "";
                if (Any(flags, UpdateFlags::UpdateDuringEditor))         strcat(buf, "Update ");
                if (Any(flags, UpdateFlags::PhysicsUpdateDuringEditor))  strcat(buf, "Physics ");
                if (Any(flags, UpdateFlags::RenderUpdateDuringEditor))   strcat(buf, "Render");
                ImGui::TextUnformatted(buf[0] ? buf : "-");
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
