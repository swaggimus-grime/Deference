#include "Common.hlsl"

struct STriVertex
{
    float3 vertex;
    float4 color;
};

StructuredBuffer<STriVertex> BTriVertex : register(t0);
StructuredBuffer<int> indices : register(t1);
RaytracingAccelerationStructure SceneBVH : register(t2);

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics =
      float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();
  // #DXR Extra: Per-Instance Data
    float3 hitColor = float3(0.6, 0.7, 0.6);
  // Shade only the first 3 instances (triangles)
    if (InstanceID() < 3)
    {

    // #DXR Extra: Per-Instance Data
        hitColor = BTriVertex[indices[vertId + 0]].color * barycentrics.x +
               BTriVertex[indices[vertId + 1]].color * barycentrics.y +
               BTriVertex[indices[vertId + 2]].color * barycentrics.z;
    }

    payload.colorAndDistance = float4(hitColor, RayTCurrent());
}
