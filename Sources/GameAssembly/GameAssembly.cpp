#include <Termina/Scripting/API/ScriptingAPI.hpp>
#include <Termina/World/ComponentRegistry.hpp>
#include <ImGui/imgui.h>

#include "FlyCamComponent.hpp" 
#include "ParticleSystem.hpp"
#include "PhysicsTestComponent.hpp"
#include "MenuComponent.hpp"
#include "GameManagerComponent.hpp"
#include "RunnerPlayerComponent.hpp"
#include "CollectibleComponent.hpp"
#include "Procedural.hpp"
#include "RunnerGameManager.hpp"
#include "CollideCheck.hpp"
#include "JumpBoostPickup.hpp"
#include "SideScrolling.hpp"
#include "GameOverComponent.hpp"


COMPONENT_MODULE_BEGIN()
    REGISTER_COMPONENT(FlyCamComponent, "Fly Cam Component")
    REGISTER_COMPONENT(ParticleSystemComponent, "Particle System")
    REGISTER_COMPONENT(PhysicsTestComponent, "Physics Test")
    REGISTER_COMPONENT(MenuComponent, "Menu")
    REGISTER_COMPONENT(GameOverComponent, "GameOver")
    REGISTER_COMPONENT(GameManagerComponent, "GameManager")
    REGISTER_COMPONENT(RunnerGameManager, "Runner Game Manager")
    REGISTER_COMPONENT(RunnerPlayerComponent, "Runner Player Component")
    REGISTER_COMPONENT(Collectibles, "Collectibles")
    REGISTER_COMPONENT(Procedural, "Procedural manager")    
    REGISTER_COMPONENT(CollideStatus, "ColliderCheck")
    REGISTER_COMPONENT(JumpBoostPickup, "Jump Boost Pickup")
    REGISTER_COMPONENT(SideScrolling, "Side Scrolling Manager")
COMPONENT_MODULE_END()

TERMINA_DLL_EXPORT void SetImGuiContext(void* ctx, void* allocFunc, void* freeFunc, void* userData)
{
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(ctx));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<ImGuiMemAllocFunc>(allocFunc),
        reinterpret_cast<ImGuiMemFreeFunc>(freeFunc),
        userData);
}
