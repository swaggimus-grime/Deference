#ifndef __LIGHT_HLSLI__
#define __LIGHT_HLSLI__

struct PointLight
{
    float3 pos;
    float intensity;
    float3 color;
    float emissive;
};

struct LightData
{
    float distToLight;
    float3 vToL;
    float3 dirToL;
};

LightData GetLightData(float3 vPos, float3 lPos)
{
    LightData data;
    float3 vToL = vPos + lPos;
    data.distToLight = length(vToL);
    data.dirToL = normalize(vToL);
    data.vToL = vToL;
    return data;
}

ConstantBuffer<PointLight> pointLight : register(b0);

#endif