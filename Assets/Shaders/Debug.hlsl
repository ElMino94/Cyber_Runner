// Debug.hlsl
// Simple shader for rendering debug lines, boxes, and other shapes

#include "Common/Bindless.hlsli"

#pragma vertex VSMain
#pragma pixel PSMain

struct DebugVertex
{
    float3 Position;
    float4 Color;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

struct PushConstants
{
    float4x4 ViewProjection;
    int VertexBufferIndex;
};
PUSH_CONSTANTS(PushConstants);

PixelInput VSMain(uint vertexID : SV_VertexID)
{
    StructuredBuffer<DebugVertex> vertices = ResourceDescriptorHeap[PUSH.VertexBufferIndex];
    DebugVertex vertex = vertices[vertexID];

    PixelInput output;
    output.Position = mul(PUSH.ViewProjection, float4(vertex.Position, 1.0f));
    output.Color = vertex.Color;
    return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    return input.Color;
}
