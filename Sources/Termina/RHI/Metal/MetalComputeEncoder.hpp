#pragma once

#include <Termina/RHI/ComputeEncoder.hpp>

#include <Metal/Metal.h>
#include "MetalRenderContext.hpp"

namespace Termina {

    class MetalComputeEncoder : public ComputeEncoder
    {
    public:
        MetalComputeEncoder(MetalRenderContext* ctx, const std::string& name, ContextToEncoder&& ctxToEnc);
        ~MetalComputeEncoder();

        void SetPipeline(Pipeline* pipeline) override;
        void SetConstants(uint32 size, const void* data) override;

        void Dispatch(uint32 x, uint32 y, uint32 z, uint32 groupSizeX, uint32 groupSizeY, uint32 groupSizeZ) override;
        void End() override;
    private:
        MetalRenderContext* m_ParentCtx;
        Pipeline* m_CurrentPipeline;

        id<MTLComputeCommandEncoder> m_CommandEncoder;
    };
} // namespace Termina
