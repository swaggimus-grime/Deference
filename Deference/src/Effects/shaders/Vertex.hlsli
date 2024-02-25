#ifndef __VERTEX_HLSLI__
#define __VERTEX_HLSLI__

struct VertexIn
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
    float3 norm : NORMAL;
    float4 tan : TANGENT;
    //float3 bitan : BITANGENT;
    //float4 color : COLOR;
};

struct VertexOut
{
    float4 pos : SV_Position;
    linear float3 wPos : POSITION;
    float2 tex : TEXCOORD;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    //float4 color : COLOR;
};

struct ScreenVertexOut
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};

#endif