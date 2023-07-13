struct VertexIn
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
    float3x3 TBN : TANGENT_SPACE;
};

struct ScreenVertexOut
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};