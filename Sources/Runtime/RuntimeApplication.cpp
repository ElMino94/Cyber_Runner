#include "RuntimeApplication.hpp"
#include "ImGui/imgui.h"
#include "Termina/Asset/AssetSystem.hpp"
#include "Termina/Input/InputSystem.hpp"
#include "Termina/Physics/PhysicsSystem.hpp"
#include "Termina/RHI/Sampler.hpp"
#include "Termina/Scripting/ScriptSystem.hpp"

#include <Termina/Renderer/Renderer.hpp>
#include <Termina/Shader/ShaderManager.hpp>
#include <Termina/Audio/AudioSystem.hpp>
#include <Termina/World/WorldSystem.hpp>
#include <Termina/World/ComponentRegistry.hpp>

RuntimeApplication::RuntimeApplication()
    : Application("Runtime")
{
    m_SystemManager.AddSystem<Termina::InputSystem>(m_Window->GetHandle());
    m_SystemManager.AddSystem<Termina::WorldSystem>();
    m_SystemManager.AddSystem<Termina::RendererSystem>(m_Window);
    m_SystemManager.AddSystem<Termina::ShaderManager>();
    m_SystemManager.AddSystem<Termina::AudioSystem>();
    m_SystemManager.AddSystem<Termina::ScriptSystem>();
    m_SystemManager.AddSystem<Termina::AssetSystem>();
    m_SystemManager.AddSystem<Termina::PhysicsSystem>();

    Termina::ComponentRegistry::Get().Report();

    Termina::WorldSystem* worldSystem = m_SystemManager.GetSystem<Termina::WorldSystem>();
    worldSystem->LoadWorld("Assets/Worlds/Sponza.trw");

    Termina::ShaderManager* shaderManager = m_SystemManager.GetSystem<Termina::ShaderManager>();
    Termina::RenderPipelineDesc rpDesc = Termina::RenderPipelineDesc().AddColorAttachmentFormat(Termina::TextureFormat::BGRA8_UNORM)
                                                                          .SetName("Triangle Pipeline")
                                                                          .SetCullMode(Termina::PipelineCullMode::NONE);
    shaderManager->GetShaderServer().WatchPipeline("Assets/Shaders/FullscreenPass.hlsl", rpDesc, Termina::PipelineType::Graphics);

    Termina::RendererSystem* rendererSystem = m_SystemManager.GetSystem<Termina::RendererSystem>();
    rendererSystem->RegisterRenderCallback([this, shaderManager, rendererSystem](Termina::RendererDevice* device, Termina::RendererSurface* surface, float){
        auto context = surface->GetContext();

        Termina::RenderEncoderInfo rei = Termina::RenderEncoderInfo()
            .SetName("Fullscreen Render")
            .SetDimensions(surface->GetWidth(), surface->GetHeight())
            .AddColorAttachment(surface->GetCurrentTextureView(), true, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        Termina::RenderEncoder* re = context->CreateRenderEncoder(rei);
        re->SetPipeline(shaderManager->GetShaderServer().GetPipeline("Assets/Shaders/FullscreenPass.hlsl"));

        Termina::TextureViewDesc viewDesc = Termina::TextureViewDesc().CreateDefault(rendererSystem->GetPassIO()->GetTexture("RendererOutput"), Termina::TextureViewType::SHADER_READ, Termina::TextureViewDimension::TEXTURE_2D);
        Termina::TextureView* outputView = rendererSystem->GetResourceViewCache()->GetTextureView(viewDesc);
        Termina::Sampler* sampler = rendererSystem->GetSamplerCache()->GetSampler(Termina::SamplerDesc().SetFilter(Termina::SamplerFilter::POINT).SetAddress(Termina::SamplerAddressMode::CLAMP));

        struct PushConstants {
            int SamplerIndex;
            int TextureIndex;
            int ReverseUV;
            int Padding;
        } pc;
        pc.SamplerIndex = sampler->GetBindlessHandle();
        pc.TextureIndex = outputView->GetBindlessIndex();
        pc.ReverseUV = true;

        re->SetConstants(sizeof(pc), &pc);
        re->SetViewport(0.0f, 0.0f, static_cast<float>(surface->GetWidth()), static_cast<float>(surface->GetHeight()));
        re->SetScissorRect(0, 0, surface->GetWidth(), surface->GetHeight());
        re->Draw(3, 1, 0, 0);
        re->End();
    });

    worldSystem->GetCurrentWorld()->OnPlay();
}

void RuntimeApplication::OnPreUpdate(float dt)
{
    Termina::WorldSystem* worldSystem = m_SystemManager.GetSystem<Termina::WorldSystem>();

    Termina::RendererSystem* renderer = Application::GetSystem<Termina::RendererSystem>();
    renderer->SetCurrentCamera(worldSystem->GetCurrentWorld()->GetMainCamera());
}
