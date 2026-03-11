#include "TrianglePass.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/Shader/ShaderManager.hpp>
#include <Termina/Shader/ShaderServer.hpp>

namespace Termina {
    TrianglePass::TrianglePass()
    {
        ShaderServer& server = Application::GetSystem<ShaderManager>()->GetShaderServer();

        RenderPipelineDesc rpDesc = RenderPipelineDesc().AddColorAttachmentFormat(TextureFormat::BGRA8_UNORM)
                                                                      .SetName("Triangle Pipeline");
        server.WatchPipeline("Assets/Shaders/Triangle.hlsl", rpDesc, PipelineType::Graphics);
    }

    void TrianglePass::Execute(RenderPassExecuteInfo& info)
    {
        ShaderServer& server = Application::GetSystem<ShaderManager>()->GetShaderServer();

        RenderEncoderInfo rei = RenderEncoderInfo().AddColorAttachment(info.Surface->GetCurrentTextureView(), true, glm::vec4(0.2f, 0.8f, 0.5f, 1.0f))
                                                   .SetName("Clear Color")
                                                   .SetDimensions(info.Width, info.Height);
        RenderEncoder* re = info.Ctx->CreateRenderEncoder(rei);
        re->SetPipeline(server.GetPipeline("Assets/Shaders/Triangle.hlsl"));
        re->SetViewport(0.0f, 0.0f, static_cast<float>(info.Width), static_cast<float>(info.Height));
        re->SetScissorRect(0.0f, 0.0f, static_cast<float>(info.Width), static_cast<float>(info.Height));
        re->Draw(3, 1, 0, 0);
        re->End();
    }
}
