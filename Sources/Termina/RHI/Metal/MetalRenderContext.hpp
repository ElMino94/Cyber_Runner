#pragma once

#include <Termina/RHI/RenderContext.hpp>

#include <Metal/Metal.h>

#include "MetalAB.hpp"

#include <vector>

namespace Termina {

    // Snapshot of the context state handed off to a newly-created encoder.
    // Encoders receive a ContextToEncoder instance and flush the contained
    // barriers inside the encoder without creating transient passes.
    struct ContextToEncoder
    {
        id<MTLFence> Fence = nil;
        std::vector<TextureBarrier> TextureBarriers;
        std::vector<BufferBarrier> BufferBarriers;
    };

    class MetalDevice;

    class MetalRenderContext : public RenderContext
    {
    public:
        MetalRenderContext(MetalDevice* device, bool allocate = true);
        ~MetalRenderContext() override;

        void Reset() override;
        void Begin() override;
        void End() override;

        void Barrier(const TextureBarrier& textureBarrier) override;
        void Barrier(const BufferBarrier& bufferBarrier) override;
        void Barrier(const BarrierGroup& barrierGroup) override;

        RenderEncoder* CreateRenderEncoder(const RenderEncoderInfo& info) override;
        CopyEncoder* CreateCopyEncoder(const std::string& name = "Copy Pass") override;
        ComputeEncoder* CreateComputeEncoder(const std::string& name = "Compute Pass") override;

        void PushMarker(const std::string& name) override;
        void PopMarker() override;

        id<MTLCommandBuffer> GetCommandBuffer() { return m_CommandBuffer; }
        MetalAB* GetAB() { return m_AB; }
        MetalDevice* GetParentDevice() { return m_ParentDevice; }

        // Acquire a snapshot of the context state (fence + pending barriers) which
        // will be moved into a newly-created encoder. The encoder is then responsible
        // for flushing those barriers inside its own encoding scope.
        ContextToEncoder AcquireContextToEncoder();

        // Legacy accessors kept for compatibility (may be used by other code).
        id<MTLFence> GetFence() { return m_EncoderFence; }

        std::vector<TextureBarrier>& GetPendingTexBarriers() { return m_PendingTexBarriers; }
        std::vector<BufferBarrier>& GetPendingBufBarriers() { return m_PendingBufBarriers; }
        void ClearPendingBarriers() { m_PendingTexBarriers.clear(); m_PendingBufBarriers.clear(); }

    private:
        id<MTLCommandBuffer> m_CommandBuffer;
        id<MTLFence> m_EncoderFence = nil;

        MetalDevice* m_ParentDevice = nullptr;
        MetalAB* m_AB = nullptr;

        std::vector<TextureBarrier> m_PendingTexBarriers;
        std::vector<BufferBarrier> m_PendingBufBarriers;
    };
} // namespace Termina
