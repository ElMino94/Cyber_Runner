#pragma once

#include <Termina/Renderer/RenderPass.hpp>
#include <Termina/RHI/Buffer.hpp>
#include <Termina/RHI/BufferView.hpp>
#include <Termina/RHI/TLAS.hpp>

namespace Termina {

    /// Compute lighting pass: reads GBuffer, evaluates PBR lights, outputs HDR color via UAV.
    /// When raytracing is available, traces inline shadow rays against the scene TLAS.
    class DeferredPass : public RenderPass
    {
    public:
        DeferredPass();
        ~DeferredPass() override;

        void Resize(int32 width, int32 height) override;
        void Execute(RenderPassExecuteInfo& info) override;

    private:
        static constexpr int32  MAX_LIGHTS         = 256;
        static constexpr uint32 MAX_TLAS_INSTANCES = 4096;

        RendererTexture* m_HDRTexture    = nullptr; // RGBA16_FLOAT
        RendererBuffer*  m_LightBuffer   = nullptr; // GPULight[MAX_LIGHTS], eCpuToGpu
        BufferView*      m_LightBufView  = nullptr; // Bindless SRV
        void*            m_LightMapped   = nullptr; // Persistent map pointer

        TLAS*            m_TLAS          = nullptr;
        RendererBuffer*  m_TLASScratch   = nullptr;
    };

} // namespace Termina
