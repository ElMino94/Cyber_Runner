#include "ResourceViewCache.h"

namespace Termina {
    ResourceViewCache::~ResourceViewCache()
    {
        Clear();
    }

    TextureView* ResourceViewCache::GetTextureView(const TextureViewDesc& desc)
    {
        uint64 key = DeriveTextureKey(desc);

        auto it = m_ResourceViews.find(key);
        if (it != m_ResourceViews.end() && it->second.Tex) {
            return it->second.Tex;
        }

        TextureView* textureView = m_Device->CreateTextureView(desc);
        ResourceViewEntry entry;
        entry.Tex = textureView;
        m_ResourceViews[key] = entry;

        return textureView;

    }
    BufferView* ResourceViewCache::GetBufferView(const BufferViewDesc& desc)
    {
        uint64 key = DeriveBufferKey(desc);

        auto it = m_ResourceViews.find(key);
        if (it != m_ResourceViews.end() && it->second.Buf) {
            return it->second.Buf;
        }

        BufferView* bufferView = m_Device->CreateBufferView(desc);
        ResourceViewEntry entry;
        entry.Buf = bufferView;
        m_ResourceViews[key] = entry;

        return bufferView;
    }

    void ResourceViewCache::Clear()
    {
        for (auto& pair : m_ResourceViews) {
            if (pair.second.Tex) {
                delete pair.second.Tex;
            }
            if (pair.second.Buf) {
                delete pair.second.Buf;
            }
        }
        m_ResourceViews.clear();
    }

    uint64 ResourceViewCache::DeriveTextureKey(const TextureViewDesc& desc)
    {
        uint64 key = 0;
        key ^= static_cast<uint64>(desc.Type) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.Format) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.MipStart) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.MipEnd) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.ArrayLayerStart) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.ArrayLayerEnd) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= reinterpret_cast<uint64>(desc.Texture) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.Dimension) + 0x9e3779b9 + (key << 6) + (key >> 2);
        return key;
    }

    uint64 ResourceViewCache::DeriveBufferKey(const BufferViewDesc& desc)
    {
        uint64 key = 0;
        key ^= static_cast<uint64>(desc.Type) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= reinterpret_cast<uint64>(desc.Buffer) + 0x9e3779b9 + (key << 6) + (key >> 2);
        return key;
    }
}
