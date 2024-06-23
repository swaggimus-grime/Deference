#ifndef __SHADING_HLSLI__
#define __SHADING_HLSLI__

#include "Vertex.hlsli"
#include "../HostDeviceShared.h"

StructuredBuffer<float3> vertices : register(t0, space1);
ByteAddressBuffer indices : register(t1, space1);
StructuredBuffer<float2> uvs_0 : register(t2, space1);
StructuredBuffer<float2> uvs_1 : register(t3, space1);
StructuredBuffer<float2> uvs_2 : register(t4, space1);
StructuredBuffer<float3> normals : register(t5, space1);
StructuredBuffer<float4> tangents : register(t6, space1);

struct MeshData
{
    uint MaterialID;
    uint Istride;
};
ConstantBuffer<MeshData> mesh : register(b0, space1);

struct TextureIndexer
{
    int ID;
    int TexCoord;
};

struct Material
{
    float4 BaseColor;
    float Roughness;
    float Metallic;
    TextureIndexer BaseTex;
    TextureIndexer RMTex;
    TextureIndexer NormTex;
    TextureIndexer OccTex;
    TextureIndexer EmTex;
    float3 EmissiveColor;
    float IoR;
    bool doubleSidedMaterial;
};

StructuredBuffer<Material> mats : register(t7, space1);
Texture2D<float4> textures[MAX_TEXTURES] : register(t0, space2);
SamplerState smp : register(s0);

struct ShadingData
{
    float3 wPos;
    float3 normal;
    float3 diffuse;
    float opacity;
    float roughness;
    float metallic;
    float3 emissive;
    float3 specular;
    float IoR;
    bool doubleSidedMaterial;
};

float2 BarycentricLerp2(in float2 v0, in float2 v1, in float2 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

float3 BarycentricLerp3(in float3 v0, in float3 v1, in float3 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

float2 getTCs(float2 uv0, float2 uv1, float2 uv2, int tc)
{        
    if (tc == 1)
        return uv1;
    else if (tc == 2)
        return uv2;
    
    return uv0;
}

ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attrib)
{
    uint address = primIdx * 3;
    uint3 idx;
    if(mesh.Istride == 2)
    {
        uint16_t3 i3 = indices.Load<uint16_t3>(address * 2);
        idx.x = i3.x;
        idx.y = i3.y;
        idx.z = i3.z;
    }
    else
    {
        uint3 i3 = indices.Load<uint3>(address * 4);
        idx.x = i3.x;
        idx.y = i3.y;
        idx.z = i3.z;
    }
    
    float3 v = BarycentricLerp3(
        vertices[idx.x],
        vertices[idx.y],
        vertices[idx.z],
        attrib
    );
    
    Material mat = mats[mesh.MaterialID];
    float2 uv0 = BarycentricLerp2(
                uvs_0[idx.x],
                uvs_0[idx.y],
                uvs_0[idx.z],
                attrib);
    float2 uv1 = BarycentricLerp2(
                uvs_1[idx.x],
                uvs_1[idx.y],
                uvs_1[idx.z],
                attrib);
    float2 uv2 = BarycentricLerp2(
                uvs_2[idx.x],
                uvs_2[idx.y],
                uvs_2[idx.z],
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
    
    {
        n = normalize(n);
        data.normal = n;
        
        if(mat.NormTex.ID != -1)
        {
            float3 norm = textures[mat.NormTex.ID].SampleLevel(smp, getTCs(uv0, uv1, uv2, mat.NormTex.TexCoord), 0).rgb;
            if (any(norm))
            {
                norm = norm * 2 - 1;
                float3 tangent = normalize(t - dot(t, n) * n);
                float3 bitan = cross(n, t);
                float3x3 texSpace = float3x3(tangent, bitan, n);
                data.normal = normalize(mul(norm, texSpace));
            }
        }
        
        data.IoR = mat.IoR;
        data.doubleSidedMaterial = mat.doubleSidedMaterial;
        if(data.doubleSidedMaterial)
            data.normal = -data.normal;
    }
    {
        if (mat.BaseTex.ID != -1)
        {
            float4 diffuse = textures[mat.BaseTex.ID].SampleLevel(smp, getTCs(uv0, uv1, uv2, mat.BaseTex.TexCoord), 0);
            data.diffuse = diffuse.rgb;
            data.opacity = diffuse.a;
        }
        else
        {
            data.diffuse = mat.BaseColor.rgb;
            data.opacity = mat.BaseColor.a;
        }
    }
    {
        if (mat.RMTex.ID != -1)
        {
            float4 rm = textures[mat.RMTex.ID].SampleLevel(smp, getTCs(uv0, uv1, uv2, mat.RMTex.TexCoord), 0);
            data.roughness = rm.r; //* rm.r;
            data.metallic = rm.g;
            data.specular = rm.rgb;
        }
    }
    {
        if (mat.EmTex.ID != -1)
        {
            float4 emissive = textures[mat.EmTex.ID].SampleLevel(smp, getTCs(uv0, uv1, uv2, mat.EmTex.TexCoord), 0);
            data.emissive = emissive.rgb;
        }
        else
        {
            data.emissive = mat.EmissiveColor;
        }
    }

    return data;
}

#endif