#include "ScriptDebug.hpp"

namespace TerminaScript {
    void Debug::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawLine(start, end, color, transform);
    }

    void Debug::DrawBox(const glm::vec3& center, const glm::vec3& extents, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawBox(center, extents, color, transform);
    }

    void Debug::DrawSphere(const glm::vec3& center, float radius, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawSphere(center, radius, segments, color, transform);
    }

    void Debug::DrawCapsule(const glm::vec3& from, const glm::vec3& to, float radius, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawCapsule(from, to, radius, segments, color, transform);
    }

    void Debug::DrawCone(const glm::vec3& apex, const glm::vec3& base, float radius, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawCone(apex, base, radius, segments, color, transform);
    }

    void Debug::DrawTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawTriangle(p0, p1, p2, color, transform);
    }

    void Debug::DrawArrow(const glm::vec3& from, const glm::vec3& to, float headSize, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawArrow(from, to, headSize, color, transform);
    }

    void Debug::DrawPlane(const glm::vec3& center, const glm::vec3& normal, float size, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawPlane(center, normal, size, color, transform);
    }

    void Debug::DrawRing(const glm::vec3& center, const glm::vec3& normal, float radius, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawRing(center, normal, radius, segments, color, transform);
    }

    void Debug::DrawCylinder(const glm::vec3& from, const glm::vec3& to, float radius, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawCylinder(from, to, radius, segments, color, transform);
    }

    void Debug::DrawAxes(const glm::vec3& origin, float scale, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawAxes(origin, scale, transform);
    }

    void Debug::DrawArc(const glm::vec3& center, const glm::vec3& normal, const glm::vec3& startDir, float radius, float angle, int32 segments, const glm::vec4& color, const glm::mat4* transform)
    {
        Termina::DebugPass::DrawArc(center, normal, startDir, radius, angle, segments, color, transform);
    }

    void Debug::DrawFrustum(const glm::mat4& viewProj, const glm::vec4& color)
    {
        Termina::DebugPass::DrawFrustum(viewProj, color);
    }

    void Debug::DrawGrid(const glm::vec3& center, const glm::vec3& normal, float size, int32 cells, const glm::vec4& color)
    {
        Termina::DebugPass::DrawGrid(center, normal, size, cells, color);
    }

}
