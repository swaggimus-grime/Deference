#include "ModelVertex.hlsli"
#include "Transform.hlsli"

VSOut main(float3 pos : POSITION/*, float2 tex : TEXCOORD, float3 norm : NORMAL*/)
{
    VSOut output;
    output.pos = float4(pos, 1.f);
    //output.tex = tex;
    //output.norm = norm;
    return output;
}