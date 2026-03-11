#pragma once

#include <Termina/Renderer/RenderPass.hpp>

namespace Termina {
    class TrianglePass : public RenderPass
    {
    public:
        TrianglePass();
        ~TrianglePass() override = default;

        void Execute(RenderPassExecuteInfo& Info) override;
    };
}
