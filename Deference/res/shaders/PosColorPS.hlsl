#include "ModelVertex.hlsli"

//SamplerState samp : register(s0);
//Texture2D dmap : register(t0);

float4 main(VSOut input) : SV_Target
{
    //return dmap.Sample(samp, input.tex);
    return input.color;
}
