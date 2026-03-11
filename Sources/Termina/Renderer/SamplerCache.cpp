#include "SamplerCache.h"

namespace Termina {
    SamplerCache::~SamplerCache()
    {
        Clear();
    }

    Sampler* SamplerCache::GetSampler(const SamplerDesc& desc)
    {
        uint64 key = DeriveSamplerKey(desc);
        if (m_Samplers.count(key) > 0)
            return m_Samplers[key];

        Sampler* sampler = m_Device->CreateSampler(desc);
        m_Samplers[key] = sampler;
        return sampler;
    }

    void SamplerCache::Clear()
    {
        for (auto [key, sampler] : m_Samplers)
            delete sampler;
        m_Samplers.clear();
    }

    uint64 SamplerCache::DeriveSamplerKey(const SamplerDesc& desc)
    {
        uint64 key = 0;
        key ^= static_cast<uint64>(desc.Address) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.ComparisonFunc) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.Filter) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= static_cast<uint64>(desc.UseMips) + 0x9e3779b9 + (key << 6) + (key >> 2);
        return key;
    }
}
