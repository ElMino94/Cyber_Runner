// FullscreenPass.hlsl

#include "Common/Bindless.hlsli"

#pragma vertex VSMain
#pragma pixel PSMain

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

struct PushConstants
{
    int SamplerIndex;
    int TextureIndex;
    int ReverseUV;
    int Padding;
};
PUSH_CONSTANTS(PushConstants);

PixelInput VSMain(uint VertexID : SV_VertexID)
{
    // Full-screen triangle: vertices at (-1,-1), (3,-1), (-1,3)
    float2 uv = float2((VertexID << 1) & 2, VertexID & 2);

    PixelInput output;
    output.Position = float4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.UV = float2(uv.x, PUSH.ReverseUV ? 1.0f - uv.y : uv.y);
    return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    SamplerState samp = SamplerDescriptorHeap[PUSH.SamplerIndex];
    Texture2D<float4> tex = ResourceDescriptorHeap[PUSH.TextureIndex];
    return tex.Sample(samp, input.UV);
}
