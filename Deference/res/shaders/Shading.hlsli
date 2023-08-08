#ifndef __SHADING_HLSLI__
#define __SHADING_HLSLI__

#include "Vertex.hlsli"

StructuredBuffer<VertexIn> gVertices : register(t0, space1);
ByteAddressBuffer gIndices : register(t1, space1);
Texture2D<float4> dmap : register(t2, space1);
Texture2D<float4> nmap : register(t3, space1);

struct ShadingData
{
    float3 wPos;
    float3 normal;
    float3 diffuse;
    float opacity;
};

float2 BarycentricLerp2(in float2 v0, in float2 v1, in float2 v2, float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float3 BarycentricLerp3(in float3 v0, in float3 v1, in float3 v2, float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

VertexIn BarycentricLerp(in VertexIn v0, in VertexIn v1, in VertexIn v2, in float3 barycentrics)
{
    VertexIn v;
    v.pos = BarycentricLerp3(v0.pos, v1.pos, v2.pos, barycentrics);
    v.norm = normalize(BarycentricLerp3(v0.norm, v1.norm, v2.norm, barycentrics));
    v.tex = BarycentricLerp2(v0.tex, v1.tex, v2.tex, barycentrics);
    v.tan = normalize(BarycentricLerp3(v0.tan, v1.tan, v2.tan, barycentrics));
    v.bitan = normalize(BarycentricLerp3(v0.bitan, v1.bitan, v2.bitan, barycentrics));

    return v;
}

ShadingData getShadingData(uint primIdx, BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    uint baseIndex = primIdx * 3;
    int address = baseIndex * 4;
    const uint3 indices = gIndices.Load3(address);
    
    VertexIn v = BarycentricLerp(
        gVertices[indices[0]],
        gVertices[indices[1]],
        gVertices[indices[2]],
        barycentrics
    );
    
    ShadingData data;
    data.normal = v.norm;
    data.wPos = v.pos;
    
    float2 texDims;
    dmap.GetDimensions(texDims.x, texDims.y);
    float4 diffuse = dmap.Load(int3(floor(v.tex * texDims), 0));
    data.diffuse = diffuse.rgb;
    data.opacity = diffuse.a;
    
    return data;
}

#endif