#include "RuntimeApplication.hpp"
#include "ImGui/imgui.h"
#include "Termina/RHI/RenderContext.hpp"
#include "Termina/RHI/RenderPipeline.hpp"
#include "Termina/RHI/Texture.hpp"

#include <Termina/Renderer/Renderer.hpp>
#include <Termina/Shader/ShaderManager.hpp>

RuntimeApplication::RuntimeApplication()
    : Application("Runtime")
{
    m_SystemManager.AddSystem<Termina::RendererSystem>(m_Window);
    m_SystemManager.AddSystem<Termina::ShaderManager>();
}

void RuntimeApplication::OnUpdate(float dt)
{
    ImGui::ShowDemoWindow();
}
