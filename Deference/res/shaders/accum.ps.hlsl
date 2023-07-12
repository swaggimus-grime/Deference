#include "Vertex.hlsli"

Texture2D<float4> gCurFrame : register(t0);
Texture2D<float4> gLastFrame : register(t1);

struct Accum
{
	uint gAccumCount; // How many frames have we already accumulated?
};

ConstantBuffer<Accum> accumConstants : register(b0);

float4 main(float4 pos : SV_Position) : SV_Target0
{
	uint2 pixelPos = (uint2) pos.xy; // Where is this pixel on screen?
	float4 curColor = gCurFrame[pixelPos]; // Pixel color this frame
	float4 prevColor = gLastFrame[pixelPos]; // Pixel color last frame

    // Do a weighted sum, weighing last frame's color based on total count
	return (accumConstants.gAccumCount * prevColor + curColor) / (accumConstants.gAccumCount + 1);
}