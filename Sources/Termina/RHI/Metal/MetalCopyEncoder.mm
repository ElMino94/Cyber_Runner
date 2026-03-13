#include "MetalCopyEncoder.hpp"
#include "MetalRenderContext.hpp"
#include "MetalTexture.hpp"
#include "MetalBuffer.hpp"

namespace Termina {
    MetalCopyEncoder::MetalCopyEncoder(MetalRenderContext* context, const std::string& name, ContextToEncoder&& ctxToEnc)
    {
        // keep reference to parent context (header already declares m_ParentCtx)
        m_ParentCtx = context;
        m_Encoder = [context->GetCommandBuffer() blitCommandEncoder];
        m_Encoder.label = name.empty() ? @"Copy Pass" : [NSString stringWithUTF8String:name.c_str()];

        // Note: MTLBlitCommandEncoder does not support memory barriers or fence operations.
        // The provided barriers and fence are ignored since blit operations don't require
        // explicit synchronization at the encoder level in Metal.
    }

    void MetalCopyEncoder::CopyBufferToBuffer(RendererBuffer* srcBuffer, uint64 srcOffset, RendererBuffer* dstBuffer, uint64 dstOffset, uint64 size)
    {
        MetalBuffer* srcMtl = (MetalBuffer*)srcBuffer;
        MetalBuffer* dstMtl = (MetalBuffer*)dstBuffer;
        [m_Encoder copyFromBuffer:srcMtl->GetBuffer()
                   sourceOffset:srcOffset
                   toBuffer:dstMtl->GetBuffer()
                   destinationOffset:dstOffset
                   size:size];
    }

    void MetalCopyEncoder::CopyBufferToBuffer(RendererBuffer* srcBuffer, RendererBuffer* dstBuffer)
    {
        MetalBuffer* srcMtl = (MetalBuffer*)srcBuffer;
        MetalBuffer* dstMtl = (MetalBuffer*)dstBuffer;
        [m_Encoder copyFromBuffer:srcMtl->GetBuffer()
                   sourceOffset:0
                   toBuffer:dstMtl->GetBuffer()
                   destinationOffset:0
                   size:srcMtl->GetSize()];
    }

    void MetalCopyEncoder::CopyBufferToTexture(RendererBuffer* srcBuffer, RendererTexture* dstTexture, const BufferTextureCopyRegion& region)
    {
        MetalBuffer* srcMtl = (MetalBuffer*)srcBuffer;
        MetalTexture* dstMtl = (MetalTexture*)dstTexture;

        MTLOrigin origin = MTLOriginMake(region.TextureOffsetX, region.TextureOffsetY, region.TextureOffsetZ);
        MTLSize size = MTLSizeMake(region.TextureExtentWidth, region.TextureExtentHeight, region.TextureExtentDepth);

        NSUInteger bytesPerPixel = TextureFormatBPP(dstTexture->GetDesc().Format);
        NSUInteger rowLengthInPixels = region.BufferRowLength > 0 ? region.BufferRowLength : size.width;
        NSUInteger bytesPerRow = rowLengthInPixels * bytesPerPixel;
        NSUInteger bytesPerImage = region.BufferImageHeight > 0
            ? region.BufferImageHeight * bytesPerRow
            : size.height * bytesPerRow;

        [m_Encoder copyFromBuffer:srcMtl->GetBuffer()
                   sourceOffset:region.BufferOffset
                   sourceBytesPerRow:bytesPerRow
                   sourceBytesPerImage:bytesPerImage
                   sourceSize:size
                   toTexture:dstMtl->GetTexture()
                   destinationSlice:region.BaseArrayLayer
                   destinationLevel:region.MipLevel
                   destinationOrigin:origin];
    }

    void MetalCopyEncoder::CopyTextureToBuffer(RendererTexture* srcTexture, RendererBuffer* dstBuffer, const BufferTextureCopyRegion& region)
    {
        MetalTexture* srcMtl = (MetalTexture*)srcTexture;
        MetalBuffer* dstMtl = (MetalBuffer*)dstBuffer;

        MTLOrigin origin = MTLOriginMake(region.TextureOffsetX, region.TextureOffsetY, region.TextureOffsetZ);
        MTLSize size = MTLSizeMake(region.TextureExtentWidth, region.TextureExtentHeight, region.TextureExtentDepth);

        // Calculate bytes per row - BufferRowLength is in pixels (0 means tightly packed)
        uint32 bytesPerPixel = TextureFormatBPP(srcTexture->GetDesc().Format);
        uint64 rowLengthInPixels = region.BufferRowLength > 0 ? region.BufferRowLength : region.TextureExtentWidth;
        uint64 bytesPerRow = rowLengthInPixels * bytesPerPixel;
        uint64 bytesPerImage = region.BufferImageHeight > 0 ? region.BufferImageHeight * bytesPerRow : region.TextureExtentHeight * bytesPerRow;

        [m_Encoder copyFromTexture:srcMtl->GetTexture()
                   sourceSlice:region.BaseArrayLayer
                   sourceLevel:region.MipLevel
                   sourceOrigin:origin
                   sourceSize:size
                   toBuffer:dstMtl->GetBuffer()
                   destinationOffset:region.BufferOffset
                   destinationBytesPerRow:bytesPerRow
                   destinationBytesPerImage:bytesPerImage];
    }

    void MetalCopyEncoder::CopyTextureToTexture(RendererTexture* srcTexture, RendererTexture* dstTexture, const TextureCopyRegion& region)
    {
        MetalTexture* srcMtl = (MetalTexture*)srcTexture;
        MetalTexture* dstMtl = (MetalTexture*)dstTexture;

        MTLOrigin srcOrigin = MTLOriginMake(region.SrcOffsetX, region.SrcOffsetY, region.SrcOffsetZ);
        MTLOrigin dstOrigin = MTLOriginMake(region.DstOffsetX, region.DstOffsetY, region.DstOffsetZ);
        MTLSize size = MTLSizeMake(region.ExtentWidth, region.ExtentHeight, region.ExtentDepth);

        [m_Encoder copyFromTexture:srcMtl->GetTexture()
                   sourceSlice:region.SrcBaseArrayLayer
                   sourceLevel:region.SrcMipLevel
                   sourceOrigin:srcOrigin
                   sourceSize:size
                   toTexture:dstMtl->GetTexture()
                   destinationSlice:region.DstBaseArrayLayer
                   destinationLevel:region.DstMipLevel
                   destinationOrigin:dstOrigin];
    }

    void MetalCopyEncoder::CopyTextureToTexture(RendererTexture* srcTexture, RendererTexture* dstTexture)
    {
        MetalTexture* srcMtl = (MetalTexture*)srcTexture;
        MetalTexture* dstMtl = (MetalTexture*)dstTexture;

        const TextureDesc& srcDesc = srcTexture->GetDesc();

        MTLOrigin origin = MTLOriginMake(0, 0, 0);

        // Copy all mip levels and array layers
        for (uint32 layer = 0; layer < srcDesc.ArrayLayers; ++layer) {
            for (uint32 mip = 0; mip < srcDesc.MipLevels; ++mip) {
                uint32 mipWidth = std::max(1u, srcDesc.Width >> mip);
                uint32 mipHeight = std::max(1u, srcDesc.Height >> mip);
                uint32 mipDepth = std::max(1u, srcDesc.Depth >> mip);
                MTLSize mipSize = MTLSizeMake(mipWidth, mipHeight, mipDepth);

                [m_Encoder copyFromTexture:srcMtl->GetTexture()
                           sourceSlice:layer
                           sourceLevel:mip
                           sourceOrigin:origin
                           sourceSize:mipSize
                           toTexture:dstMtl->GetTexture()
                           destinationSlice:layer
                           destinationLevel:mip
                           destinationOrigin:origin];
            }
        }
    }

    void MetalCopyEncoder::ResetBuffer(RendererBuffer* buffer, uint64 offset, uint64 size)
    {
        MetalBuffer* mtlBuffer = (MetalBuffer*)buffer;
        [m_Encoder fillBuffer:mtlBuffer->GetBuffer()
                   range:NSMakeRange(offset, size)
                   value:0];
    }

    void MetalCopyEncoder::End()
    {
        // Note: MTLBlitCommandEncoder does not support fence operations.
        // No fence update is performed here.
        [m_Encoder endEncoding];
        delete this;
    }
} // namespace Termina
