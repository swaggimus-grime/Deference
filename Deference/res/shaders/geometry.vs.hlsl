#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), modelTransform.mvp);
    output.tex = input.tex;
    output.wPos = mul(float4(input.pos, 1.f), modelTransform.model).xyz;
    output.norm = mul(input.norm, (float3x3) modelTransform.model);
    output.tan = mul(input.tan, (float3x3) modelTransform.model);
    output.color = input.color;
    return output;
}