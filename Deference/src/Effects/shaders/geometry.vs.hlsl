#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), mul(modelTransform.model, modelTransform.vp));
    output.tex = input.tex;
    output.wPos = mul(float4(input.pos, 1.f), modelTransform.model).xyz;
    output.norm = mul(input.norm.xyz, (float3x3) modelTransform.model);
    output.tan = mul(input.tan.xyz, (float3x3) modelTransform.model);
    return output;
}