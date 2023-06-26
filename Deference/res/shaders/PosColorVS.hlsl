#include "ModelVertex.hlsli"
#include "Transform.hlsli"

VSOut main(float3 pos : POSITION, float2 tex : TEXCOORD, float4 color : COLOR)
{
    VSOut output;
    output.pos = mul(float4(pos, 1.f), transform.view);
    output.pos = mul(output.pos, transform.proj);
    output.tex = tex;
    output.color = color;
    return output;
}