#include "..\\HostDeviceShared.h"
#include "Vertex.hlsli"

Texture2D<float4> textures[MAX_TEXTURES] : register(t0, space1);
TextureCube skyBox : register(t0, space2);

SamplerState smp : register(s0);

struct Camera
{
    float3 pos;
};
ConstantBuffer<Camera> cam : register(b0);

struct Light
{
    float3 pos;
    float intensity;
    float3 ambient;
    float specPower;
    float3 color;
};
ConstantBuffer<Light> light : register(b1);

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
ConstantBuffer<Material> mat : register(b2);

float3 diffuse(float3 albedo, float3 lightColor, float NdotL)
{
    return albedo * (lightColor * max(NdotL, 0.0f));
}

float3 fresnel(float m, float3 lightColor, float3 F0, float NdotH, float NdotL, float LdotH)
{
    float3 R = F0 + (1.0f - F0) * (1.0f - max(LdotH, 0.0f));
    R = R * ((m + 8) / 8) * pow(NdotH, m);
    return saturate(lightColor * max(NdotL, 0.0f) * R);
}

float4 main(VertexOut input) : SV_Target
{
    float4 baseColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 normal = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 roughMetallic = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 occlusion = { 1.0f, 1.0f, 1.0f, 1.0f };
    float4 emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    if (mat.BaseTex != -1)
        baseColor = textures[mat.BaseTex].Sample(smp, input.tex);
    if (mat.NormTex != -1)
        normal = textures[mat.NormTex].Sample(smp, input.tex);
    if (mat.RMTex != -1)
        roughMetallic = textures[mat.RMTex].Sample(smp, input.tex);
    if (mat.OccTex != -1)
        occlusion = textures[mat.OccTex].Sample(smp, input.tex);
    if (mat.EmTex != -1)
        emissive = textures[mat.EmTex].Sample(smp, input.tex);
    
    input.norm = normalize(input.norm);

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
    
    float3 V = normalize(cam.pos - input.wPos);
    if (dot(input.norm, V) < 0)
        input.norm = -input.norm;
    
    float3 L = normalize(light.pos - input.wPos);
    float3 H = normalize(L + V); // Half vector between light and viewer
    float3 N = input.norm;
    
    float3 R = reflect(-V, N);
    
    float VdotH = dot(V, H);
    float LdotH = dot(L, H);
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    float NdotH = dot(N, H);

    float3 dielectricSpecular = { 0.04f, 0.04f, 0.04f };
    float3 black = { 0.0f, 0.0f, 0.0f };
    float4 cubeMapSample = skyBox.Sample(smp, R);
    float roughness = roughMetallic.g * mat.Roughness;
    float metallic = roughMetallic.b * mat.Metallic;
    float3 albedo = baseColor.rgb;

    float3 C_diff = lerp(baseColor.rgb * (1.0f - dielectricSpecular), black, metallic);
    float3 F0 = lerp(dielectricSpecular, baseColor.rgb, metallic);

    float3 C_ambient = light.ambient * albedo * occlusion.x;
    float3 C_diffuse = diffuse(C_diff, light.color, NdotL) * light.intensity;

    float m = metallic * light.specPower; // Exponent in the model for the roughness. Higher the value, more the material is shine.
    float3 C_specular = fresnel(m, light.color, F0, NdotH, NdotL, LdotH);
    float3 f = C_ambient + C_diffuse + C_specular + emissive.xyz + cubeMapSample.xyz;

    return float4(f, 1.0f);
}