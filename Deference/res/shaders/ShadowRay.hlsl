#include "Common.hlsl"

// #DXR Extra - Another ray type
// Ray payload for the shadow rays
[shader("closesthit")]
void ShadowClosestHit(inout ShadowHitInfo hit, Attributes bary)
{
    hit.isHit = true;
}

[shader("miss")]
void ShadowMiss(inout ShadowHitInfo hit : SV_RayPayload)
{
    hit.isHit = false;
}