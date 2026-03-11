#pragma once

#include <Termina/RHI/Device.hpp>

namespace Termina {
    class GPUBumpAllocator
    {
    public:
        GPUBumpAllocator(RendererDevice* device, uint64 totalSize);
        ~GPUBumpAllocator();

        uint64 Allocate(uint64 size, uint64 alignment);
        void Reset();

        void* GetPointerAtOffset(uint64 offset) const { return static_cast<uint8*>(m_MappedPtr) + offset; }
        RendererBuffer* GetBuffer() const { return m_Buffer; }
    private:
        RendererBuffer* m_Buffer;
        void* m_MappedPtr;
        uint64 m_TotalSize;
        uint64 m_Offset;
    };
}
