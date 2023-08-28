#include "Vertex.hlsli"

struct GeometryBuffer
{
    float4 worldPos : SV_Target0;
    float4 worldNorm : SV_Target1;
    float4 worldDiff : SV_Target2;
    float4 worldSpec : SV_Target3;
};

Texture2D<float4> dmap : register(t0);
Texture2D<float4> nmap : register(t1);
Texture2D<float4> smap : register(t2);

SamplerState smp : register(s0);

GeometryBuffer main(VertexOut input)
{
    float3 diff = dmap.Sample(smp, input.tex).rgb;

    float3 norm;
    norm = nmap.Sample(smp, input.tex).rgb;
    norm = norm * 2 - 1;
    norm = normalize(mul(norm, input.TBN));
    
    float4 spec;
    spec = smap.Sample(smp, input.tex);
    
    GeometryBuffer gbuff;
    gbuff.worldPos = input.pos;
    gbuff.worldNorm = float4(norm, 1.f);
    gbuff.worldDiff = float4(diff, 1.f);
    gbuff.worldSpec = float4(spec.rgb, spec.a);
    
    return gbuff;
}
