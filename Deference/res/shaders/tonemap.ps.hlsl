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

// John Hable's Uncharted 2 filmic tone map
// http://filmicgames.com/archives/75
float3 ApplyUc2Curve(float3 color)
{
    float A = 0.22; //Shoulder Strength
    float B = 0.3; //Linear Strength
    float C = 0.1; //Linear Angle
    float D = 0.2; //Toe Strength
    float E = 0.01; //Toe Numerator
    float F = 0.3; //Toe Denominator

    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - (E / F);
    return color;
}

float4 main(ScreenVertexOut v) : SV_Target0
{
    uint2 pixelPos = (uint2) v.pos.xy;
    float4 color = hdr[pixelPos];
    if(options.on)
        return float4(ApplyUc2Curve(color.rgb), color.a);
    return color;
}