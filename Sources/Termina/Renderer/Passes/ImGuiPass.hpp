#pragma once

#include <Termina/Renderer/RenderPass.hpp>

#include <ImGui/imgui.h>

namespace Termina {
    class ImGuiPass : public RenderPass
    {
    public:
        ImGuiPass();
        ~ImGuiPass();

        void Execute(RenderPassExecuteInfo& Info) override;
    
    private:
        void UpdateTexture(ImDrawData* data, RenderPassExecuteInfo& Info);

        struct ImGuiFontTexture {
            RendererTexture* Texture;
            TextureView* View;
            bool UploadedAtLeastOnce;
        };

        struct FrameResource {
            RendererBuffer* VertexBuffer = nullptr;
            BufferView* VertexBufferView = nullptr;
            int32 VertexBufferSize = 10000;

            RendererBuffer* IndexBuffer = nullptr;
            int32 IndexBufferSize = 5000;
        };

        Sampler* m_FontSampler = nullptr;

        std::vector<FrameResource> m_FrameResources;
    };
}
