#include "ModelVertex.hlsli"

//SamplerState samp : register(s0);
//Texture2D dmap : register(t0);

float4 main(VSOut input) : SV_Target
{
    //return dmap.Sample(samp, input.tex);
    return float4(1.f, 1.f, 0.f, 1.f);
}
