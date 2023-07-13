#include "Vertex.hlsli"
#include "Transform.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.pos = mul(float4(input.pos, 1.f), modelTransform.mvp);
    
    float3 T = normalize(mul(input.tan, modelTransform.normMat).xyz);
    float3 N = normalize(mul(input.norm, modelTransform.normMat).xyz);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = transpose(float3x3(T, B, N));
    output.TBN = TBN;
    output.tex = input.tex;
    return output;
}