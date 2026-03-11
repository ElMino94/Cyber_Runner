// ImGui.hlsl

#include "Common/Bindless.hlsli"

#pragma vertex VSMain
#pragma pixel PSMain

struct VertexInput
{
    float2 Position : POSITION;
    float2 UV : TEXCOORD;
    uint Color : COLOR;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

struct PushConstants
{
    float4x4 ProjectionMatrix;

    int VertexIndex;
    int SamplerIndex;
    int TextureIndex;
    int VertexOffset;
};
PUSH_CONSTANTS(PushConstants);

float4 UnpackColor(uint u)
{
    float4 color;
    color.r = (u & 0xFF) / 255.0;
    color.g = ((u >> 8) & 0xFF) / 255.0;
    color.b = ((u >> 16) & 0xFF) / 255.0;
    color.a = ((u >> 24) & 0xFF) / 255.0;
    return color;
}

PixelInput VSMain(uint vertexID : SV_VertexID)
{
    StructuredBuffer<VertexInput> vertices = ResourceDescriptorHeap[PUSH.VertexIndex];
    VertexInput input = vertices[vertexID + PUSH.VertexOffset];

    float4 color = UnpackColor(input.Color);

    PixelInput output;
    output.Position = mul(PUSH.ProjectionMatrix, float4(input.Position, 0.f, 1.f));
    output.Color = color;
    output.UV = input.UV;
    return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    SamplerState sampler = SamplerDescriptorHeap[PUSH.SamplerIndex];
    Texture2D<float4> texture = ResourceDescriptorHeap[PUSH.TextureIndex];

    float4 texColor = texture.Sample(sampler, input.UV);
    float4 outColor = input.Color * texColor;
    return outColor;
}
