#include "ImGuiPass.hpp"
#include "Renderer/RenderPass.hpp"

#include <Termina/Renderer/Renderer.hpp>
#include <Termina/Core/Application.hpp>
#include <Termina/Shader/ShaderManager.hpp>
#include <Termina/Shader/ShaderServer.hpp>

namespace Termina {
    struct ImGuiPushConstants
    {
        glm::mat4 ProjectionMatrix;

        int VertexIndex;
        int SamplerIndex;
        int TextureIndex;
        int VertexOffset;
    };

    ImGuiPass::ImGuiPass()
    {
        RendererDevice* device = Application::GetSystem<RendererSystem>()->GetDevice();
        ShaderServer& server = Application::GetSystem<ShaderManager>()->GetShaderServer();

        RenderPipelineDesc pipelineDesc = RenderPipelineDesc()
                                     .SetCullMode(PipelineCullMode::NONE)
                                     .SetEnableBlending(true)
                                     .AddColorAttachmentFormat(TextureFormat::BGRA8_UNORM);
        server.WatchPipeline("Assets/Shaders/ImGui.hlsl", pipelineDesc, PipelineType::Graphics);

        m_FontSampler = device->CreateSampler(SamplerDesc()
                                        .SetFilter(SamplerFilter::LINEAR)
                                        .SetAddress(SamplerAddressMode::CLAMP));
        m_FrameResources.resize(FRAMES_IN_FLIGHT);
    }

    ImGuiPass::~ImGuiPass()
    {
        for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
            if (tex->RefCount == 1) {
                ImGuiFontTexture* texture = (ImGuiFontTexture*)tex->BackendUserData;

                delete texture->Texture;
                delete texture->View;
                delete texture;

                tex->SetTexID(ImTextureID_Invalid);
                tex->SetStatus(ImTextureStatus_Destroyed);
                tex->BackendUserData = nullptr;
            }
        }
        for (uint64 i = 0; i < m_FrameResources.size(); i++) {
            FrameResource& resource = m_FrameResources[i];

            delete resource.VertexBufferView;
            delete resource.VertexBuffer;
            delete resource.IndexBuffer;
        }
        delete m_FontSampler;
    }

    void ImGuiPass::Execute(RenderPassExecuteInfo& info)
    {
        ImGui::Render();
        ImDrawData* data = ImGui::GetDrawData();
        UpdateTexture(data, info);

        uint32 frameIndex = info.FrameIndex;
        FrameResource& resource = m_FrameResources[frameIndex];
        if (resource.VertexBuffer == nullptr || resource.VertexBufferSize < data->TotalVtxCount) {
            if (resource.VertexBufferView) delete resource.VertexBufferView;
            if (resource.VertexBuffer) delete resource.VertexBuffer;

            resource.VertexBufferSize = data->TotalVtxCount + 5000;
            resource.VertexBuffer = info.Device->CreateBuffer(BufferDesc()
                                                            .SetSize(resource.VertexBufferSize * sizeof(ImDrawVert))
                                                            .SetUsage(BufferUsage::SHADER_READ | BufferUsage::TRANSFER)
                                                            .SetStride(sizeof(ImDrawVert)));
            resource.VertexBuffer->SetName("ImGui Vertex Buffer");
            resource.VertexBufferView = info.Device->CreateBufferView(BufferViewDesc()
                                                                  .SetBuffer(resource.VertexBuffer)
                                                                  .SetType(BufferViewType::SHADER_READ));
        }
        if (resource.IndexBuffer == nullptr || resource.IndexBufferSize < data->TotalIdxCount) {
            if (resource.IndexBuffer) delete resource.IndexBuffer;

            resource.IndexBufferSize = data->TotalIdxCount + 10000;
            resource.IndexBuffer = info.Device->CreateBuffer(BufferDesc()
                                                           .SetSize(resource.IndexBufferSize * sizeof(ImDrawIdx))
                                                           .SetUsage(BufferUsage::INDEX | BufferUsage::TRANSFER)
                                                           .SetStride(sizeof(ImDrawIdx)));
            resource.IndexBuffer->SetName("ImGui Index Buffer");
        }

        void* vtx_resource = resource.VertexBuffer->Map();
        void* idx_resource = resource.IndexBuffer->Map();

        ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
        ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;

        for (int n = 0; n < data->CmdListsCount; n++) {
            const ImDrawList* drawList = data->CmdLists[n];
            memcpy(vtx_dst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += drawList->VtxBuffer.Size;
            idx_dst += drawList->IdxBuffer.Size;
        }

        resource.VertexBuffer->Unmap();
        resource.IndexBuffer->Unmap();

        float L = data->DisplayPos.x;
        float R = data->DisplayPos.x + data->DisplaySize.x;
        float T = data->DisplayPos.y;
        float B = data->DisplayPos.y + data->DisplaySize.y;
        glm::mat4 mvp = {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };

        ImGuiPushConstants pushConstants = {};
        pushConstants.ProjectionMatrix = mvp;
        pushConstants.VertexIndex = resource.VertexBufferView->GetBindlessHandle();
        pushConstants.SamplerIndex = m_FontSampler->GetBindlessHandle();

        ShaderServer& server = Application::GetSystem<ShaderManager>()->GetShaderServer();

        RenderPipeline* pipeline = server.GetPipeline("Assets/Shaders/ImGui.hlsl");

        RenderEncoderInfo encoderInfo = RenderEncoderInfo().AddColorAttachment(info.Surface->GetCurrentTextureView(), false)
                                                           .SetDimensions(info.Width, info.Height)
                                                           .SetName("ImGui Pass");

        RenderEncoder* encoder = info.Ctx->CreateRenderEncoder(encoderInfo);
        encoder->SetPipeline(pipeline);
        encoder->SetIndexBuffer(resource.IndexBuffer);
        encoder->SetViewport(0.0f, 0.0f, data->DisplaySize.x * data->FramebufferScale.x, data->DisplaySize.y * data->FramebufferScale.y);

        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        ImVec2 clip_off = data->DisplayPos;
        ImVec2 clip_scale = data->FramebufferScale;
        for (int n = 0; n < data->CmdListsCount; n++) {
            const ImDrawList* drawList = data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < drawList->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &drawList->CmdBuffer[cmd_i];
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp scissor rect to viewport bounds
                float viewport_width = data->DisplaySize.x * data->FramebufferScale.x;
                float viewport_height = data->DisplaySize.y * data->FramebufferScale.y;

                if (clip_min.x < 0.0f) clip_min.x = 0.0f;
                if (clip_min.y < 0.0f) clip_min.y = 0.0f;
                if (clip_max.x > viewport_width) clip_max.x = viewport_width;
                if (clip_max.y > viewport_height) clip_max.y = viewport_height;

                // Skip draw call if scissor rect is invalid
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                pushConstants.TextureIndex = (int32)pcmd->GetTexID();
                pushConstants.VertexOffset = pcmd->VtxOffset + global_vtx_offset;

                // Apply scissor/clipping rectangle, bind, draw
                encoder->SetScissorRect((int)clip_min.x, (int)clip_min.y, (int)clip_max.x, (int)clip_max.y);
                encoder->SetConstants(sizeof(pushConstants), &pushConstants);
                encoder->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, 0, 0);
            }
            global_idx_offset += drawList->IdxBuffer.Size;
            global_vtx_offset += drawList->VtxBuffer.Size;
        }
        encoder->End();
    }

    void ImGuiPass::UpdateTexture(ImDrawData* data, RenderPassExecuteInfo& info)
    {
        if (data->Textures != nullptr) {
            for (ImTextureData* tex : *data->Textures) {
                if (tex->Status != ImTextureStatus_OK) {
                    if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 2) {
                        ImGuiFontTexture* texture = static_cast<ImGuiFontTexture*>(tex->BackendUserData);

                        delete texture->Texture;
                        delete texture->View;
                        delete texture;

                        tex->SetTexID(ImTextureID_Invalid);
                        tex->SetStatus(ImTextureStatus_Destroyed);
                        tex->BackendUserData = nullptr;
                    }
                    if (tex->Status == ImTextureStatus_WantCreate) {
                        ImGuiFontTexture* texture = new ImGuiFontTexture();

                        TextureDesc desc = TextureDesc()
                                           .SetSize(tex->Width, tex->Height)
                                           .SetFormat(TextureFormat::RGBA8_UNORM)
                                           .SetUsage(TextureUsage::SHADER_READ);
                        texture->Texture = info.Device->CreateTexture(desc);
                        texture->Texture->SetName("ImGui Font Texture");

                        TextureViewDesc viewDesc = TextureViewDesc()
                                                  .SetFormat(TextureFormat::RGBA8_UNORM)
                                                  .SetType(TextureViewType::SHADER_READ)
                                                  .SetDimension(TextureViewDimension::TEXTURE_2D)
                                                  .SetMipRange(0, 1)
                                                  .SetArrayLayerRange(0, 1)
                                                  .SetTexture(texture->Texture);
                        texture->View = info.Device->CreateTextureView(viewDesc);

                        texture->UploadedAtLeastOnce = false;

                        tex->SetTexID(texture->View->GetBindlessIndex());
                        tex->SetStatus(ImTextureStatus_WantUpdates);
                        tex->BackendUserData = texture;
                    }
                    if (tex->Status == ImTextureStatus_WantUpdates) {
                        ImGuiFontTexture* texture = static_cast<ImGuiFontTexture*>(tex->BackendUserData);

                        TextureUploadDesc desc;
                        desc.Width = tex->Width;
                        desc.Height = tex->Height;
                        desc.Depth = 1;
                        desc.ArrayLayer = 0;
                        desc.MipLevel = 0;
                        desc.Format = TextureFormat::RGBA8_UNORM;

                        info.Uploader->QueueTextureUpload(texture->Texture, tex->GetPixels(), tex->GetSizeInBytes(), desc);

                        tex->SetStatus(ImTextureStatus_OK);
                    }
                }
            }
        }
    }
}