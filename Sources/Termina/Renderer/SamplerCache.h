#pragma once

#include <Termina/RHI/Device.hpp>

#include <unordered_map>

namespace Termina
{
    using SamplerKey = uint64;

    class SamplerCache
    {
    public:
        SamplerCache(RendererDevice* device)
            : m_Device(device)
        {
        }
        ~SamplerCache();

        Sampler* GetSampler(const SamplerDesc& desc);

        void Clear();
    public:
        inline uint64 DeriveSamplerKey(const SamplerDesc& desc);

    private:
        RendererDevice* m_Device = nullptr;

        std::unordered_map<uint64, Sampler*> m_Samplers;
    };
}
