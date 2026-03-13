#include "MetalRenderContext.hpp"
#include "MetalRenderEncoder.hpp"
#include "MetalCopyEncoder.hpp"
#include "MetalDevice.hpp"
#include "MetalComputeEncoder.hpp"
#include "MetalTexture.hpp"
#include "MetalBuffer.hpp"

namespace Termina {

    // Acquire a snapshot of the context state to hand off to a newly-created encoder.
    // This moves the pending barrier lists and the current fence into a ContextToEncoder
    // instance so encoders can flush them inside their own encoding scope without the
    // context creating any transient encoders.
    ContextToEncoder MetalRenderContext::AcquireContextToEncoder()
    {
        ContextToEncoder snap;
        snap.Fence = m_EncoderFence;

        // Move pending lists into the snapshot to transfer ownership to the encoder.
        snap.TextureBarriers = std::move(m_PendingTexBarriers);
        snap.BufferBarriers = std::move(m_PendingBufBarriers);

        // Clear the context's containers to reflect the transfer of ownership.
        m_PendingTexBarriers.clear();
        m_PendingBufBarriers.clear();

        return snap;
    }

    MetalRenderContext::MetalRenderContext(MetalDevice* device, bool allocate)
        : m_ParentDevice(device)
    {
        if (!allocate) return;
        m_CommandBuffer = [device->GetCommandQueue() commandBuffer];

        // Create an encoder fence which encoders will wait/update when flushing
        // pending render-context level barriers into encoders. This mirrors the
        // pattern used in the old RHI where fences are per-command-list/ctx.
        m_EncoderFence = [device->GetDevice() newFence];

        m_AB = new MetalAB(device, 65536);
    }

    MetalRenderContext::~MetalRenderContext()
    {
        // Clear the encoder fence so any retained ObjC reference is released.
        m_EncoderFence = nil;

        if (m_AB) delete m_AB;
    }

    void MetalRenderContext::Reset()
    {
        if (!m_AB) m_AB = new MetalAB(m_ParentDevice, 65536);
        m_AB->Reset();
        m_CommandBuffer = [m_ParentDevice->GetCommandQueue() commandBuffer];

        // Create a fresh fence for this command buffer to track encoder synchronization.
        // This ensures barriers from one frame don't interfere with the next.
        m_EncoderFence = [m_ParentDevice->GetDevice() newFence];

        [m_ParentDevice->GetResidencySet() commit];
    }

    void MetalRenderContext::Begin()
    {
    }

    void MetalRenderContext::End()
    {
        // Don't nil the command buffer here — it must remain valid for
        // ExecuteRenderContext() to commit it. Reset() will allocate a fresh one.
    }

    void MetalRenderContext::Barrier(const TextureBarrier& textureBarrier)
    {
        // Record the texture barrier at the render-context level to be flushed
        // into the next created encoder.
        m_PendingTexBarriers.push_back(textureBarrier);
    }

    void MetalRenderContext::Barrier(const BufferBarrier& bufferBarrier)
    {
        // Record buffer barrier for later flush.
        m_PendingBufBarriers.push_back(bufferBarrier);
    }

    void MetalRenderContext::Barrier(const BarrierGroup& barrierGroup)
    {
        // Append all texture and buffer barriers from the group.
        for (const auto& tb : barrierGroup.TextureBarriers) {
            m_PendingTexBarriers.push_back(tb);
        }
        for (const auto& bb : barrierGroup.BufferBarriers) {
            m_PendingBufBarriers.push_back(bb);
        }
    }

    RenderEncoder* MetalRenderContext::CreateRenderEncoder(const RenderEncoderInfo& info)
    {
        // Acquire snapshot and pass it to the encoder so it can flush the barriers
        // at the start of its encoding scope (encoder will not create a transient pass).
        ContextToEncoder snapshot = AcquireContextToEncoder();
        return new MetalRenderEncoder(this, info, std::move(snapshot));
    }

    CopyEncoder* MetalRenderContext::CreateCopyEncoder(const std::string& name)
    {
        // Acquire snapshot and hand it to the copy encoder so it can flush barriers
        // inside its own blit encoder at the beginning.
        ContextToEncoder snapshot = AcquireContextToEncoder();
        return new MetalCopyEncoder(this, name, std::move(snapshot));
    }

    ComputeEncoder* MetalRenderContext::CreateComputeEncoder(const std::string& name)
    {
        // Acquire snapshot and pass it to the compute encoder so it can flush the
        // pending barriers at the start of its compute encoding (no transient passes).
        ContextToEncoder snapshot = AcquireContextToEncoder();
        return new MetalComputeEncoder(this, name, std::move(snapshot));
    }

    void MetalRenderContext::PushMarker(const std::string& name)
    {
        (void)name;
        // TODO: Pass to current encoder?
    }

    void MetalRenderContext::PopMarker()
    {
    }
} // namespace Termina
