#include "MetalComputeEncoder.hpp"
#include "MetalRenderContext.hpp"
#include "MetalComputePipeline.hpp"
#include "MetalDevice.hpp"
#include "MetalTexture.hpp"
#include "MetalBuffer.hpp"

#include <Termina/Core/Logger.hpp>
#include <MetalShaderConverter/metal_irconverter_runtime.h>

namespace Termina {
    MetalComputeEncoder::MetalComputeEncoder(MetalRenderContext* ctx, const std::string& name, ContextToEncoder&& ctxToEnc)
        : m_ParentCtx(ctx)
    {
        // Create the compute encoder as usual
        m_CommandEncoder = [m_ParentCtx->GetCommandBuffer() computeCommandEncoder];
        m_CommandEncoder.label = [NSString stringWithUTF8String:name.c_str()];

        // Wait on the provided fence (if any) so previously flushed work is visible.
        if (ctxToEnc.Fence) {
            [m_CommandEncoder waitForFence:ctxToEnc.Fence];
        }

        // Collect textures referenced by the context snapshot and issue barriers on this encoder.
        std::vector<id<MTLTexture>> textures;
        textures.reserve(ctxToEnc.TextureBarriers.size());
        for (const auto& tb : ctxToEnc.TextureBarriers) {
            if (tb.TargetTexture) {
                MetalTexture* mt = reinterpret_cast<MetalTexture*>(tb.TargetTexture);
                if (mt && mt->GetTexture()) textures.push_back(mt->GetTexture());
            }
        }
        if (!textures.empty()) {
            [m_CommandEncoder memoryBarrierWithResources:textures.data() count:textures.size()];
        }

        // Collect buffers referenced by the context snapshot and issue barriers on this encoder.
        std::vector<id<MTLBuffer>> buffers;
        buffers.reserve(ctxToEnc.BufferBarriers.size());
        for (const auto& bb : ctxToEnc.BufferBarriers) {
            if (bb.TargetBuffer) {
                MetalBuffer* mb = reinterpret_cast<MetalBuffer*>(bb.TargetBuffer);
                if (mb && mb->GetBuffer()) buffers.push_back(mb->GetBuffer());
            }
        }
        if (!buffers.empty()) {
            [m_CommandEncoder memoryBarrierWithResources:buffers.data() count:buffers.size()];
        }

        // Do NOT update the fence here; End() will update the context fence to mark this encoder's work as complete.
    }

    MetalComputeEncoder::~MetalComputeEncoder()
    {
        // Destructor intentionally does not update the context fence. End()
        // is responsible for updating the fence and ending the encoding. The
        // destructor is left empty to avoid double-ending encoders when End()
        // is called by the owner and then delete is invoked.
    }

    void MetalComputeEncoder::SetPipeline(Pipeline* pipeline)
    {
        if (pipeline->GetType() != PipelineType::Compute) TN_ERROR("Cannot bind a non-compute pipeline to a compute encoder!");
        MetalComputePipeline* pipe = static_cast<MetalComputePipeline*>(pipeline);

        MetalDevice* device = m_ParentCtx->GetParentDevice();
        m_CurrentPipeline = pipeline;

        [m_CommandEncoder setComputePipelineState:pipe->GetPipelineState()];

        id<MTLBuffer> descriptorHeap = device->GetBindlessManager()->GetHandle();
        id<MTLBuffer> samplerHeap = device->GetBindlessManager()->GetSamplerHandle();

        [m_CommandEncoder setBuffer:descriptorHeap offset:0 atIndex:kIRDescriptorHeapBindPoint];
        [m_CommandEncoder setBuffer:samplerHeap offset:0 atIndex:kIRSamplerHeapBindPoint];
        [m_CommandEncoder setBuffer:m_ParentCtx->GetAB()->GetBuffer() offset:0 atIndex:kIRArgumentBufferBindPoint];
    }

    void MetalComputeEncoder::SetConstants(uint32 size, const void* data)
    {
        auto alloc = m_ParentCtx->GetAB()->Alloc(1);
        memcpy(alloc.first, data, size);

        if (alloc.second != 0) {
            [m_CommandEncoder setBufferOffset:alloc.second atIndex:kIRArgumentBufferBindPoint];
        }
    }

    void MetalComputeEncoder::Dispatch(uint32 x, uint32 y, uint32 z, uint32 gx, uint32 gy, uint32 gz)
    {
        [m_CommandEncoder dispatchThreads:MTLSizeMake(x, y, z) threadsPerThreadgroup:MTLSizeMake(gx, gy, gz)];
    }

    void MetalComputeEncoder::End()
    {
        // Update the context fence to mark this encoder's work as complete before ending.
        id<MTLFence> fence = m_ParentCtx->GetFence();
        if (fence) {
            [m_CommandEncoder updateFence:fence];
        }
        [m_CommandEncoder endEncoding];
    }
} // namespace Termina
