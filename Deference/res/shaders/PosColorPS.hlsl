#include "ModelVertex.hlsli"

SamplerState samp : register(s0);
Texture2D dmap : register(t0);

float4 main(VSOut input) : SV_Target
{
    float3 ambient = float3(0.3, 0.2, 0.4);
    float3 diffuse = dmap.Sample(samp, input.tex).rgb;
    return float4((ambient + diffuse), 1.f) * input.color;
}
