#include "Vertex.hlsli"

ScreenVertexOut main(float3 pos : POSITION, float2 tex : TEXCOORD) 
{
    ScreenVertexOut v;
    v.pos = float4(pos, 1.f);
    v.tex = tex;
    return v;
}