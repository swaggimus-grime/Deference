#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), modelTransform.mvp);
    
    float3 T = normalize(mul(float4(input.tan, 0.f), modelTransform.model).xyz);
    float3 N = normalize(mul(float4(input.norm, 0.f), modelTransform.model).xyz);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    output.norm = mul(input.norm, modelTransform.normMat);
    output.TBN = TBN;
    output.tex = input.tex;
    return output;
}