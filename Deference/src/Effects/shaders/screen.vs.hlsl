#include "Vertex.hlsli"

ScreenVertexOut main(float3 pos : POSITION0, float2 tex : TEXCOORD0) 
{
    ScreenVertexOut v;
    v.pos = float4(pos, 1.f);
    v.tex = tex;
    return v;
}