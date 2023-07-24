#include "Vertex.hlsli"

struct GeometryBuffer
{
    float4 worldPos : SV_Target0;
    float4 worldNorm : SV_Target1;
    float4 worldDiff : SV_Target2;
};

Texture2D<float4> textures[] : register(t0, space0);
SamplerState smp : register(s0);

struct Material
{
    int diffMapIndex;
    int specMapIndex;
    int normMapIndex;
};

ConstantBuffer<Material> material : register(b0);

GeometryBuffer main(VertexOut input)
{
    float3 diff = float3(0, 0, 0);
    {
        uint diffIdx = material.diffMapIndex;
        if (diffIdx != -1)
        {
            Texture2D diffTex = textures[diffIdx];
            diff = diffTex.Sample(smp, input.tex).rgb;
        }
    }

    float3 norm = float3(0, 0, 0);
    {
        uint normIdx = material.normMapIndex;
        if (normIdx != -1)
        {
            Texture2D normMap = textures[normIdx];
            norm = normMap.Sample(smp, input.tex).rgb;
            norm = norm * 2 - 1;
            norm = normalize(mul(norm, input.TBN));
        }
    }
    
    GeometryBuffer gbuff;
    gbuff.worldPos = input.pos;
    gbuff.worldNorm = float4(norm, 1.f);
    gbuff.worldDiff = float4(diff, 1.f);
    
    return gbuff;
}
