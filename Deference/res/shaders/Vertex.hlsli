struct VertexIn
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
    float3 norm : NORMAL;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
    float3 norm : NORMAL;
};