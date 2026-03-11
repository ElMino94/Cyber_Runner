#include "GPUUploader.h"

#include <Termina/Core/Logger.hpp>

#include <cstring>

// ===== GPUUploader =====

namespace Termina {
	GPUUploader::GPUUploader(RendererDevice* device, uint32 frameCount)
		: m_Device(device)
	{
		m_Frames.resize(frameCount);
		m_FlushContext = m_Device->CreateRenderContext(true);
	}

	GPUUploader::~GPUUploader()
	{
		for (UploadFrame& frame : m_Frames) {
			FreeStaging(frame.StagingToFree);
			for (UploadCommand& cmd : frame.Commands) {
				delete cmd.StagingBuffer;
			}
		}
		if (m_FlushContext) {
			delete m_FlushContext;
		}
	}

	void GPUUploader::FreeStaging(std::vector<RendererBuffer*>& buffers)
	{
		for (RendererBuffer* buf : buffers) {
			delete buf;
		}
		buffers.clear();
	}

	// ===== Queue API =====

	void GPUUploader::QueueTextureUpload(RendererTexture* dst, const void* data,
	                                     uint64 dataSize, const TextureUploadDesc& desc)
	{
		if (!dst || !data || dataSize == 0) {
			TN_WARN("Invalid texture upload parameters");
			return;
		}

		// One staging buffer per upload — simple, no sharing, no stomps
		BufferDesc bufDesc;
		bufDesc.SetSize(dataSize).SetUsage(BufferUsage::TRANSFER);
		RendererBuffer* staging = m_Device->CreateBuffer(bufDesc);
		void* mapped = staging->Map();
		memcpy(mapped, data, dataSize);
		staging->Unmap();

		UploadCommand cmd;
		cmd.CommandType = UploadCommand::TEXTURE_COPY;
		cmd.StagingBuffer = staging;
		cmd.Texture.DstTexture = dst;
		cmd.Texture.Desc = desc;

		m_Frames[m_CurrentFrame].Commands.push_back(cmd);
	}

	void GPUUploader::QueueBufferUpload(RendererBuffer* dst, const void* data,
	                                    uint64 dataSize, uint64 dstOffset)
	{
		if (!dst || !data || dataSize == 0) {
			TN_WARN("Invalid buffer upload parameters");
			return;
		}

		BufferDesc bufDesc;
		bufDesc.SetSize(dataSize).SetUsage(BufferUsage::TRANSFER);
		RendererBuffer* staging = m_Device->CreateBuffer(bufDesc);
		void* mapped = staging->Map();
		memcpy(mapped, data, dataSize);
		staging->Unmap();

		UploadCommand cmd;
		cmd.CommandType = UploadCommand::BUFFER_COPY;
		cmd.StagingBuffer = staging;
		cmd.Buffer.DstBuffer = dst;
		cmd.Buffer.DstOffset = dstOffset;
		cmd.Buffer.Size = dataSize;

		m_Frames[m_CurrentFrame].Commands.push_back(cmd);
	}

	// ===== Frame lifecycle =====

	void GPUUploader::BeginFrame(uint32 frameIndex)
	{
		m_CurrentFrame = frameIndex;
		UploadFrame& frame = m_Frames[frameIndex];

		// Free staging buffers from the last time this frame slot was used.
		// Safe because Surface::BeginFrame waited for this slot's fence.
		FreeStaging(frame.StagingToFree);
	}

	void GPUUploader::RecordUploads(RenderContext* context)
	{
		UploadFrame& frame = m_Frames[m_CurrentFrame];
		if (frame.Commands.empty())
			return;

		RecordCommands(context, frame.Commands);

		// Move staging buffers to the free-later list. They'll be deleted
		// when BeginFrame cycles back to this slot (after fence wait).
		for (UploadCommand& cmd : frame.Commands) {
			frame.StagingToFree.push_back(cmd.StagingBuffer);
		}
		frame.Commands.clear();
	}

	// ===== Synchronous flush =====

	void GPUUploader::Flush()
	{
		UploadFrame& frame = m_Frames[m_CurrentFrame];
		if (frame.Commands.empty())
			return;

		m_FlushContext->Reset();
		m_FlushContext->Begin();
		RecordCommands(m_FlushContext, frame.Commands);
		m_FlushContext->End();
		m_Device->ExecuteRenderContext(m_FlushContext);
		m_Device->WaitIdle();

		// GPU is done — free staging immediately
		for (UploadCommand& cmd : frame.Commands) {
			delete cmd.StagingBuffer;
		}
		frame.Commands.clear();
	}

	// ===== Internal: record barriers + copies =====

	void GPUUploader::RecordCommands(RenderContext* context, std::vector<UploadCommand>& commands)
	{
		// Pre-copy barriers
		for (const UploadCommand& cmd : commands) {
			if (cmd.CommandType == UploadCommand::TEXTURE_COPY) {
				TextureBarrier barrier;
				barrier.SetTargetTexture(cmd.Texture.DstTexture)
				       .SetDstStage(PipelineStage::COPY)
				       .SetDstAccess(ResourceAccess::TRANSFER_WRITE)
				       .SetNewLayout(TextureLayout::TRANSFER_DST)
				       .SetMipLevels(cmd.Texture.Desc.MipLevel, 1)
				       .SetArrayLayers(cmd.Texture.Desc.ArrayLayer, 1);
				context->Barrier(barrier);
			}
		}

		// Copy commands
		CopyEncoder* encoder = context->CreateCopyEncoder("GPU Uploads");
		for (const UploadCommand& cmd : commands) {
			if (cmd.CommandType == UploadCommand::TEXTURE_COPY) {
				BufferTextureCopyRegion region;
				region.SetBufferOffset(0)
				      .SetBufferRowLength(cmd.Texture.Desc.Width)
				      .SetBufferImageHeight(cmd.Texture.Desc.Height)
				      .SetMipLevel(cmd.Texture.Desc.MipLevel)
				      .SetBaseArrayLayer(cmd.Texture.Desc.ArrayLayer)
				      .SetLayerCount(1)
				      .SetTextureOffset(0, 0, 0)
				      .SetTextureExtent(cmd.Texture.Desc.Width, cmd.Texture.Desc.Height, cmd.Texture.Desc.Depth);
				encoder->CopyBufferToTexture(cmd.StagingBuffer, cmd.Texture.DstTexture, region);
			} else {
				encoder->CopyBufferToBuffer(cmd.StagingBuffer, 0,
				                            cmd.Buffer.DstBuffer, cmd.Buffer.DstOffset,
				                            cmd.Buffer.Size);
			}
		}
		encoder->End();

		// Post-copy barriers
		for (const UploadCommand& cmd : commands) {
			if (cmd.CommandType == UploadCommand::TEXTURE_COPY) {
				TextureBarrier barrier;
				barrier.SetTargetTexture(cmd.Texture.DstTexture)
				       .SetDstStage(PipelineStage::PIXEL_SHADER)
				       .SetDstAccess(ResourceAccess::SHADER_READ)
				       .SetNewLayout(TextureLayout::READ_ONLY)
				       .SetMipLevels(cmd.Texture.Desc.MipLevel, 1)
				       .SetArrayLayers(cmd.Texture.Desc.ArrayLayer, 1);
				context->Barrier(barrier);
			} else {
				BufferBarrier barrier;
				barrier.SetTargetBuffer(cmd.Buffer.DstBuffer)
				       .SetDstStage(PipelineStage::ALL_COMMANDS)
				       .SetDstAccess(ResourceAccess::SHADER_READ);
				context->Barrier(barrier);
			}
		}
	}
}
