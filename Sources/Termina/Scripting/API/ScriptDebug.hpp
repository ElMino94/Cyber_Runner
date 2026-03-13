#pragma once

#include <Termina/Renderer/Passes/DebugPass.hpp>

namespace TerminaScript {
    class Debug
    {
    public:
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
    };
}
