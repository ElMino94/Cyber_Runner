#pragma once

#include <Termina/Renderer/RenderPass.hpp>
#include <GLM/glm.hpp>
#include <vector>

namespace Termina {
    /// A debug renderer pass that collects debug draw calls and renders them all at once.
    /// This pass owns a vertex buffer (line topology) and allows static debug draw functions
    /// to push vertices into it, then flushes everything in a single draw call.
    class DebugPass : public RenderPass
    {
    public:
        DebugPass();
        ~DebugPass() override;

        void Execute(RenderPassExecuteInfo& info) override;
        void Resize(int32 width, int32 height) override;

        // Static debug draw functions - these accumulate vertices in the internal buffer
        // All functions accept an optional transform matrix that will be applied to the vertices
        static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawBox(const glm::vec3& center, const glm::vec3& extents, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawSphere(const glm::vec3& center, float radius, int32 segments = 16, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawCapsule(const glm::vec3& from, const glm::vec3& to, float radius, int32 segments = 8, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawCone(const glm::vec3& apex, const glm::vec3& base, float radius, int32 segments = 16, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawArrow(const glm::vec3& from, const glm::vec3& to, float headSize = 0.1f, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawPlane(const glm::vec3& center, const glm::vec3& normal, float size, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawRing(const glm::vec3& center, const glm::vec3& normal, float radius, int32 segments = 32, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawCylinder(const glm::vec3& from, const glm::vec3& to, float radius, int32 segments = 16, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawAxes(const glm::vec3& origin, float scale = 0.1f, const glm::mat4* transform = nullptr);
        static void DrawArc(const glm::vec3& center, const glm::vec3& normal, const glm::vec3& startDir, float radius, float angle, int32 segments = 16, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4* transform = nullptr);
        static void DrawFrustum(const glm::mat4& viewProj, const glm::vec4& color = glm::vec4(1.0f));
        static void DrawGrid(const glm::vec3& center, const glm::vec3& normal, float size, int32 cells, const glm::vec4& color = glm::vec4(1.0f));

        // Clear all accumulated vertices (called automatically at end of each frame)
        static void Clear();
        static DebugPass* GetInstance();

    private:
        struct DebugVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;
        };

        struct FrameResource
        {
            RendererBuffer* VertexBuffer = nullptr;
            BufferView* VertexBufferView = nullptr;
            uint32 VertexBufferSize = 0;
        };

        static DebugPass* s_Instance;
        static std::vector<DebugVertex> s_Vertices;
        static constexpr size_t MAX_DEBUG_VERTICES = 1000000; // 1 million vertices max

        std::vector<FrameResource> m_FrameResources;
        Sampler* m_Sampler = nullptr;

        void AddVertex(const glm::vec3& position, const glm::vec4& color);
        void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, const glm::mat4* transform = nullptr);
        static glm::vec3 TransformPosition(const glm::vec3& position, const glm::mat4* transform);
    };
}