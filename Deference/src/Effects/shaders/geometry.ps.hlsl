#include "Vertex.hlsli"

#define MAX_TEXTURES 4

struct GeometryBuffer
{
    float4 worldPos : SV_Target0;
    float4 worldNorm : SV_Target1;
    float4 worldDiff : SV_Target2;
    float4 worldSpec : SV_Target3;
    float4 worldEmissive : SV_Target4;
};

Texture2D<float4> textures[MAX_TEXTURES] : register(t0, space1);

SamplerState smp : register(s0);

struct Camera
{
    float3 pos;
};
ConstantBuffer<Camera> cam : register(b0);

struct Material
{
    float4 BaseColor;
    float Roughness;
    float Metallic;
    uint BaseTex;
    uint RMTex;
    uint NormTex;
    uint OccTex;
    uint EmTex;
};
ConstantBuffer<Material> mat : register(b1);

GeometryBuffer main(VertexOut input)
{
    input.norm = normalize(input.norm);
    
    float3 diff = textures[mat.BaseTex].Sample(smp, input.tex).rgb;
    if(!any(diff))
        diff = mat.BaseColor.rgb;

    {
        float3 norm;
        norm = textures[mat.NormTex].Sample(smp, input.tex).rgb;
        if (any(norm))
        {
            norm = norm * 2 - 1;
            input.tan = normalize(input.tan - dot(input.tan, input.norm) * input.norm);
            float3 bitan = cross(input.norm, input.tan);
            float3x3 texSpace = float3x3(input.tan, bitan, input.norm);
            input.norm = normalize(mul(norm, texSpace));
        }
    }
    
    float3 v = normalize(cam.pos - input.wPos);
    if (dot(input.norm, v) < 0)
        input.norm = -input.norm;
    
    float4 spec;
    spec = textures[mat.RMTex].Sample(smp, input.tex);
    
    float3 em = textures[mat.EmTex].Sample(smp, input.tex).rgb;
    
    GeometryBuffer gbuff;
    gbuff.worldPos = float4(input.wPos, 1.f);
    gbuff.worldNorm = float4(input.norm, 1.f);
    gbuff.worldDiff = float4(diff, 1.f);
    gbuff.worldSpec = float4(spec.rgb, spec.a);
    gbuff.worldEmissive = float4(em, 1.f);
    
    return gbuff;
}
