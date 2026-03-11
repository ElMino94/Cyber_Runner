#pragma once

#include <Termina/RHI/Device.hpp>

namespace Termina {
    class TemporaryContext
    {
    public:
        TemporaryContext(RendererDevice* parentDevice);
        ~TemporaryContext();
    
        RenderContext* Request();
        void Flush();
    private:
        RendererDevice* m_ParentDevice = nullptr;
    
        RenderContext* m_ActiveContext = nullptr;
        bool m_ContextInUse = false;
    };
}
