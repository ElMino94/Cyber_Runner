#include "TemporaryContext.h"

#include <Termina/Core/Logger.hpp>

namespace Termina {
    TemporaryContext::TemporaryContext(RendererDevice* parentDevice)
        : m_ParentDevice(parentDevice)
    {
    }

    TemporaryContext::~TemporaryContext()
    {
        if (m_ContextInUse) TN_ERROR("TemporaryContext destroyed while context is still in use!");
        delete m_ActiveContext;
    }

    RenderContext* TemporaryContext::Request()
    {
        if (!m_ContextInUse) {
            if (!m_ActiveContext) m_ActiveContext = m_ParentDevice->CreateRenderContext(false);
            m_ActiveContext->Reset();
            m_ActiveContext->Begin();
            m_ContextInUse = true;
        }

        return m_ActiveContext;
    }

    void TemporaryContext::Flush()
    {
        m_ActiveContext->End();
        m_ParentDevice->ExecuteRenderContext(m_ActiveContext);
        m_ParentDevice->WaitIdle();
        m_ContextInUse = false;
    }
}
