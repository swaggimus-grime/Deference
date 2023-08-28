#include "Vertex.hlsli"

Texture2D<float4> hdr : register(t0);
SamplerState smp : register(s0);

struct Options
{
    bool on;
};
ConstantBuffer<Options> options : register(b0);

float calcLuminance(float3 color)
{
    return dot(color.xyz, float3(0.299f, 0.587f, 0.114f));
}

float3 reinhardToneMap(float3 color)
{
    float luminance = calcLuminance(color);
    float reinhard = luminance / (luminance + 1);
    return color * (reinhard / luminance);
}

float4 main(ScreenVertexOut v) : SV_Target0
{
    uint2 pixelPos = (uint2) v.pos.xy;
    float4 color = hdr[pixelPos];
    if(options.on)
        return float4(reinhardToneMap(color.rgb), color.a);
    return color;
}