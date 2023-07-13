#include "Vertex.hlsli"

Texture2D<float4> gPos : register(t0);
Texture2D<float4> gDirectLighting : register(t1);
Texture2D<float4> gAccum : register(t2);

float4 main(ScreenVertexOut v) : SV_Target0
{
    uint2 pixelPos = (uint2) v.pos.xy;
    float4 worldPos = gPos[pixelPos];
    float4 directLighting = gDirectLighting[pixelPos];
    float4 accum = gAccum[pixelPos];
    float ambient = 0.1;

    float3 shadeColor;

	// Todo: reflection
    shadeColor = (directLighting * (float4(ambient, ambient, ambient, ambient) + accum)).rgb;
    bool isGeometry = (worldPos.w != 0.0f);
    shadeColor = isGeometry ? shadeColor : shadeColor + float3(0.48, 0.75, 0.85);
    return float4(shadeColor, 1.0f);
    //return accum;
}