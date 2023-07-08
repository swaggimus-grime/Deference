#include "Vertex.hlsli"

struct GeometryBuffer
{
    float4 worldPos : SV_Target0;
    float4 worldNorm : SV_Target1;
    float4 worldDiff : SV_Target2;
};

Texture2D<float4> diffTextures[] : register(t0, space0);
SamplerState smp : register(s0);

struct Material
{
    uint diffTexIndex;
};

ConstantBuffer<Material> material : register(b0);

GeometryBuffer main(VertexOut input)
{
    uint diffIdx = material.diffTexIndex;
    float3 diff = float3(1, 0, 0);
    if(diffIdx != 9999)
    {
        Texture2D diffTex = diffTextures[diffIdx];
        diff = diffTex.Sample(smp, input.tex).rgb;
    }
    
    GeometryBuffer gbuff;
    gbuff.worldPos = input.pos;
    gbuff.worldNorm = float4(input.norm, 1.f);
    gbuff.worldDiff = float4(diff, 1.f);
    
    return gbuff;
}
