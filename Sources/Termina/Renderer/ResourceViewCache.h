#pragma once

#include <Termina/RHI/Device.hpp>

#include <unordered_map>

namespace Termina {
    using ResourceViewKey = uint64;

    class ResourceViewCache
    {
    public:
        ResourceViewCache(RendererDevice* device)
            : m_Device(device)
        {
        }
        ~ResourceViewCache();

        TextureView* GetTextureView(const TextureViewDesc& desc);
        BufferView* GetBufferView(const BufferViewDesc& desc);

        void Clear();
    public:
        inline uint64 DeriveTextureKey(const TextureViewDesc& desc);
        inline uint64 DeriveBufferKey(const BufferViewDesc& desc);

    private:
        RendererDevice* m_Device = nullptr;

        struct ResourceViewEntry
        {
            TextureView* Tex = nullptr;
            BufferView* Buf = nullptr;
        };

        std::unordered_map<uint64, ResourceViewEntry> m_ResourceViews;
    };
}
