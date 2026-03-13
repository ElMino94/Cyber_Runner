#pragma once

#include <Termina/RHI/CopyEncoder.hpp>

#include <Metal/Metal.h>

#include "MetalRenderContext.hpp"

namespace Termina {
    // MetalRenderContext is included above; use the concrete ContextToEncoder type
    // defined in that header rather than a forward-declaration.
    
    class MetalCopyEncoder : public CopyEncoder
    {
    public:
        MetalCopyEncoder(MetalRenderContext* context, const std::string& name, ContextToEncoder&& ctxToEnc);
        ~MetalCopyEncoder() override = default;

        void CopyBufferToBuffer(RendererBuffer* srcBuffer, uint64 srcOffset,
                            RendererBuffer* dstBuffer, uint64 dstOffset,
                            uint64 size) override;
        void CopyBufferToBuffer(RendererBuffer* srcBuffer,
                            RendererBuffer* dstBuffer) override;
        void CopyBufferToTexture(RendererBuffer* srcBuffer,
                             RendererTexture* dstTexture,
                             const BufferTextureCopyRegion& region) override;
        void CopyTextureToBuffer(RendererTexture* srcTexture,
                             RendererBuffer* dstBuffer,
                             const BufferTextureCopyRegion& region) override;
        void CopyTextureToTexture(RendererTexture* srcTexture,
                              RendererTexture* dstTexture,
                              const TextureCopyRegion& region) override;
        void CopyTextureToTexture(RendererTexture* srcTexture,
                              RendererTexture* dstTexture) override;
        void ResetBuffer(RendererBuffer* buffer, uint64 offset, uint64 size) override;
        void End() override;

    private:
        MetalRenderContext* m_ParentCtx;
        id<MTLBlitCommandEncoder> m_Encoder;
    };
} // namespace Termina
