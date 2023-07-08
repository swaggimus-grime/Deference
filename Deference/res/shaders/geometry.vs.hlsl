#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), modelTransform.mvp);
    output.norm = mul(input.norm, modelTransform.normMat);
    output.tex = input.tex;
    return output;
}