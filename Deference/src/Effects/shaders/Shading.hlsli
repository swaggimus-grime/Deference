#ifndef __SHADING_HLSLI__
#define __SHADING_HLSLI__

#include "Vertex.hlsli"
#include "../HostDeviceShared.h"

StructuredBuffer<float3> vertices : register(t0, space1);
ByteAddressBuffer indices : register(t1, space1);
StructuredBuffer<float2> uvs : register(t2, space1);
StructuredBuffer<float3> normals : register(t3, space1);
StructuredBuffer<float4> tangents : register(t4, space1);

struct MeshData
{
    uint MaterialID;
    uint Istride;
};
ConstantBuffer<MeshData> mesh : register(b0, space1);

struct Material
{
    float4 BaseColor;
    float Roughness;
    float Metallic;
    int BaseTex;
    int RMTex;
    int NormTex;
    int OccTex;
    int EmTex;
};
StructuredBuffer<Material> mats : register(t5, space1);
Texture2D<float4> textures[MAX_TEXTURES] : register(t0, space2);
SamplerState smp : register(s0);

struct ShadingData
{
    float3 wPos;
    float3 normal;
    float3 diffuse;
    float opacity;
    float3 specular;
    float roughness;
    float3 emissive;
};

float2 BarycentricLerp2(in float2 v0, in float2 v1, in float2 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

float3 BarycentricLerp3(in float3 v0, in float3 v1, in float3 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attrib)
{
    uint address = primIdx * 3;
    uint3 idx;
    if(mesh.Istride == 2)
    {
        uint16_t3 i3 = indices.Load < uint16_t3 > (address * 2);
        idx.x = i3.x;
        idx.y = i3.y;
        idx.z = i3.z;
    }
    else
    {
        idx = indices.Load(address * 4);
    }
    
    float3 v = BarycentricLerp3(
        vertices[idx.x],
        vertices[idx.y],
        vertices[idx.z],
        attrib
    );
    
    float2 uv = BarycentricLerp2(
        uvs[idx.x],
        uvs[idx.y],
        uvs[idx.z],
        attrib);
    
    float3 n = BarycentricLerp3(
        normals[idx.x],
        normals[idx.y],
        normals[idx.z],
        attrib
    );
    
    float3 t = BarycentricLerp3(
        tangents[idx.x].xyz,
        tangents[idx.y].xyz,
        tangents[idx.z].xyz,
        attrib
    );
    
    ShadingData data;
    data.wPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();// mul(v, ObjectToWorld3x4());
   
    Material mat = mats[mesh.MaterialID];
    
    {
        n = normalize(n);
        data.normal = n;
        
        if(mat.NormTex != -1)
        {
            float3 norm = textures[mat.NormTex].SampleLevel(smp, uv, 0).rgb;
            if (any(norm))
            {
                norm = norm * 2 - 1;
                float3 tangent = normalize(t - dot(t, n) * n);
                float3 bitan = cross(n, t);
                float3x3 texSpace = float3x3(tangent, bitan, n);
                data.normal = normalize(mul(norm, texSpace));
            }
        }
    }
    {
        if (mat.BaseTex != -1)
        {
            float4 diffuse = textures[mat.BaseTex].SampleLevel(smp, uv, 0);
            data.diffuse = diffuse.rgb;
            data.opacity = diffuse.a;
        }
    }
    {
        if(mat.RMTex != -1)
        {
            float4 specular = textures[mat.RMTex].SampleLevel(smp, uv, 0);
            data.specular = specular.rgb;
            data.roughness = specular.a * specular.a;
        }
    }
    {
        if(mat.EmTex != -1)
        {
            float4 emissive = textures[mat.EmTex].SampleLevel(smp, uv, 0);
            data.emissive = emissive.rgb;
        }
    }

    return data;
}

#endif