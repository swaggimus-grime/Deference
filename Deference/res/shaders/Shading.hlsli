#ifndef __SHADING_HLSLI__
#define __SHADING_HLSLI__

#include "Vertex.hlsli"

StructuredBuffer<VertexIn> gVertices : register(t0, space1);
ByteAddressBuffer gIndices : register(t1, space1);
Texture2D<float4> dmap : register(t2, space1);
Texture2D<float4> nmap : register(t3, space1);
Texture2D<float4> smap : register(t4, space1);

struct ShadingData
{
    float3 wPos;
    float3 normal;
    float3 diffuse;
    float opacity;
    float3 specular;
    float roughness;
};

float2 BarycentricLerp2(in float2 v0, in float2 v1, in float2 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

float3 BarycentricLerp3(in float3 v0, in float3 v1, in float3 v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    return v0 + attribs.barycentrics.x * (v1 - v0) + attribs.barycentrics.y * (v2 - v0);
}

VertexIn BarycentricLerp(in VertexIn v0, in VertexIn v1, in VertexIn v2, in BuiltInTriangleIntersectionAttributes attribs)
{
    VertexIn v;
    v.pos = BarycentricLerp3(v0.pos, v1.pos, v2.pos, attribs);
    v.norm = normalize(BarycentricLerp3(v0.norm, v1.norm, v2.norm, attribs));
    v.tex = BarycentricLerp2(v0.tex, v1.tex, v2.tex, attribs);
    v.tan = normalize(BarycentricLerp3(v0.tan, v1.tan, v2.tan, attribs));
    v.bitan = normalize(BarycentricLerp3(v0.bitan, v1.bitan, v2.bitan, attribs));

    return v;
}

ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attribs)
{
    uint baseIndex = primIdx * 3;
    int address = baseIndex * 4;
    const uint3 indices = gIndices.Load3(address);
    
    VertexIn v = BarycentricLerp(
        gVertices[indices.x],
        gVertices[indices.y],
        gVertices[indices.z],
        attribs
    );
    
    ShadingData data;
    data.wPos = v.pos; //mul(v.pos, ObjectToWorld3x4());
    data.normal = v.norm;
    
    float2 texDims;
    dmap.GetDimensions(texDims.x, texDims.y);
    float4 diffuse = dmap.Load(int3(floor(v.tex * texDims), 0));
    data.diffuse = diffuse.rgb;
    data.opacity = diffuse.a;
    
    smap.GetDimensions(texDims.x, texDims.y);
    float4 specular = smap.Load(int3(floor(v.tex * texDims), 0));
    data.specular = specular.rgb;
    data.roughness = specular.a * specular.a;
    
    return data;
}

#endif