#include "WorldHierarchyPanel.hpp"

#include "ImGui/imgui.h"
#include "Termina/Renderer/UIUtils.hpp"
#include "ContentViewerPanel.hpp"
#include <Termina/Core/Application.hpp>
#include <Termina/World/WorldSystem.hpp>
#include <Termina/World/World.hpp>
#include <Termina/World/Actor.hpp>

void WorldHierarchyPanel::DrawActorNode(Termina::Actor* actor)
{
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (actor->GetChildren().empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (m_Context.ItemToInspect == actor)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (!actor->IsActive())
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

    bool open = ImGui::TreeNodeEx(
        (void*)(uintptr_t)actor->GetID(),
        flags, "%s", actor->GetName().c_str());

    if (!actor->IsActive())
        ImGui::PopStyleColor();

    // Drag source
    Termina::UIUtils::ActorPickerSource(actor);

    // Drop target: reparent dragged actor as child of this one, or spawn prefab
    Termina::UIUtils::AcceptActor([actor](Termina::Actor* dragged) {
        if (dragged != actor && !actor->IsDescendantOf(dragged))
            actor->AttachChild(dragged);
    });

    Termina::UIUtils::AcceptAsset([actor](const std::string& path) {
        if (path.find(".trp") != std::string::npos)
        {
            auto* spawned = actor->GetParentWorld()->SpawnActorFromJSON(path);
            if (spawned)
                actor->AttachChild(spawned);
        }
    });

    if (ImGui::IsItemClicked())
        m_Context.ItemToInspect = actor;

    bool shouldDestroy = false;
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Spawn Child"))
        {
            auto* child = actor->GetParentWorld()->SpawnActor();
            actor->AttachChild(child);
        }
        if (ImGui::MenuItem("Destroy"))
        {
            if (m_Context.ItemToInspect == actor)
                m_Context.ItemToInspect = nullptr;
            shouldDestroy = true;
        }
        ImGui::EndPopup();
    }

    // Destroy and TreePop must happen outside the popup's window context.
    if (shouldDestroy)
    {
        actor->GetParentWorld()->DestroyActor(actor);
        if (open) ImGui::TreePop();
        return;
    }

    if (open)
    {
        for (auto* child : actor->GetChildren())
            DrawActorNode(child);
        ImGui::TreePop();
    }
}

void WorldHierarchyPanel::OnImGuiRender()
{
    Termina::UIUtils::BeginEditorWindow(m_Name.c_str(), &m_Open);

    auto* worldSystem = Termina::Application::GetSystem<Termina::WorldSystem>();
    Termina::World* world = worldSystem ? worldSystem->GetCurrentWorld() : nullptr;

    if (!world)
    {
        ImGui::TextDisabled("No world loaded");
        Termina::UIUtils::EndEditorWindow();
        return;
    }

    ImGui::TextDisabled("%s", world->GetName().c_str());

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Create").x - ImGui::GetStyle().FramePadding.x * 2.0f);
    Termina::UIUtils::PushStylized();
    if (ImGui::Button("Create"))
        ImGui::OpenPopup("##create_actor");
    Termina::UIUtils::PopStylized();

    if (ImGui::BeginPopup("##create_actor"))
    {
        if (ImGui::MenuItem("Empty"))
            world->SpawnActor();
        ImGui::EndPopup();
    }

    ImGui::Separator();

    for (auto* root : world->GetRootActors())
        DrawActorNode(root);

    // Drop target on empty space: demote dragged actor to root level or spawn prefab.
    ImVec2 remaining = ImGui::GetContentRegionAvail();
    if (remaining.y > 0.0f)
    {
        ImGui::InvisibleButton("##root_drop", remaining);
        
        Termina::UIUtils::AcceptActor([](Termina::Actor* dragged) {
            dragged->DetachFromParent();
        });

        Termina::UIUtils::AcceptAsset([world](const std::string& path) {
            if (path.find(".trp") != std::string::npos)
            {
                world->SpawnActorFromJSON(path);
            }
        });
    }



    Termina::UIUtils::EndEditorWindow();
}
