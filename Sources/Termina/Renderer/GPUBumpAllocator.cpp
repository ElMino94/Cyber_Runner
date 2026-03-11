#include "GPUBumpAllocator.h"
#include "Renderer/Renderer.hpp"

#include <Termina/Core/Application.hpp>
#include <Termina/Core/Logger.hpp>

#include <cstring>

namespace Termina {
    GPUBumpAllocator::GPUBumpAllocator(RendererDevice* device, uint64 totalSize)
        : m_TotalSize(totalSize)
        , m_Offset(0)
    {
        BufferDesc desc = BufferDesc().SetSize(totalSize)
                                      .SetStride(0)
                                      .SetUsage(BufferUsage::CONSTANT);
        m_Buffer = device->CreateBuffer(desc);
        m_Buffer->SetName("GPUBumpAllocator::m_Buffer");

        m_MappedPtr = m_Buffer->Map();
        memset(m_MappedPtr, 0, totalSize);
    }

    GPUBumpAllocator::~GPUBumpAllocator()
    {
        m_Buffer->Unmap();
        delete m_Buffer;
    }

    uint64 GPUBumpAllocator::Allocate(uint64 size, uint64 alignment)
    {
        uint64 currentOffset = m_Offset;
        uint64 alignedOffset = AlignUp(currentOffset, alignment);
        uint64 newOffset = alignedOffset + size;
        if (newOffset > m_TotalSize) {
            TN_ERROR("GPUBumpAllocator: Out of memory");
            return UINT64_MAX;
        }

        m_Offset = newOffset;
        return alignedOffset;
    }

    void GPUBumpAllocator::Reset()
    {
        m_Offset = 0;
    }
}
