#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), modelTransform.mvp);
    
    float3 T = normalize(mul(input.tan, (float3x3) modelTransform.model).xyz);
    float3 N = normalize(mul(input.norm, (float3x3) modelTransform.model).xyz);
    float3 B = normalize(mul(input.bitan, (float3x3) modelTransform.model).xyz);
    float3x3 TBN = transpose(float3x3(T, B, N));
    output.TBN = TBN;
    output.tex = input.tex;
    return output;
}