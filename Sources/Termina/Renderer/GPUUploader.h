#pragma once

#include <Termina/RHI/Device.hpp>
#include <Termina/RHI/RenderContext.hpp>
#include <Termina/RHI/Texture.hpp>
#include <Termina/RHI/Buffer.hpp>
#include <vector>

namespace Termina {
	struct TextureUploadDesc
	{
		uint32 MipLevel = 0;
		uint32 ArrayLayer = 0;
		uint32 Width;
		uint32 Height;
		uint32 Depth = 1;
		TextureFormat Format;
	};

	class GPUUploader
	{
	public:
		GPUUploader(RendererDevice* device, uint32 frameCount);
		~GPUUploader();

		// --- Queue API (call anytime) ---
		void QueueTextureUpload(RendererTexture* dst, const void* data,
		                         uint64 dataSize, const TextureUploadDesc& desc);
		void QueueBufferUpload(RendererBuffer* dst, const void* data,
		                        uint64 dataSize, uint64 dstOffset = 0);

		// --- Frame lifecycle (called by RendererSubsystem) ---
		void BeginFrame(uint32 frameIndex);
		void RecordUploads(RenderContext* context);

		// --- Synchronous flush ---
		void Flush();

	private:
		struct UploadCommand
		{
			enum Type : uint8 { BUFFER_COPY, TEXTURE_COPY };
			Type CommandType = BUFFER_COPY;

			// Each upload gets its own staging buffer
			RendererBuffer* StagingBuffer = nullptr;

			struct BufferData
			{
				RendererBuffer* DstBuffer = nullptr;
				uint64 DstOffset = 0;
				uint64 Size = 0;
			};

			struct TextureData
			{
				RendererTexture* DstTexture = nullptr;
				TextureUploadDesc Desc = {};
			};

			BufferData Buffer;
			TextureData Texture;
		};

		struct UploadFrame
		{
			std::vector<UploadCommand> Commands;
			// Staging buffers from previous frame, pending deletion after fence
			std::vector<RendererBuffer*> StagingToFree;
		};

		void RecordCommands(RenderContext* context, std::vector<UploadCommand>& commands);
		void FreeStaging(std::vector<RendererBuffer*>& buffers);

		RendererDevice* m_Device;
		std::vector<UploadFrame> m_Frames;
		uint32 m_CurrentFrame = 0;

		RenderContext* m_FlushContext = nullptr;
	};
}
