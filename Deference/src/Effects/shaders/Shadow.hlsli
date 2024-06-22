#ifndef __SHADOW_HLSLI__
#define __SHADOW_HLSLI__

struct ShadowRayPayload
{
    float hitDist;
};

[shader("anyhit")]
void ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    rayData.hitDist = RayTCurrent();
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload rayData)
{
    rayData.hitDist = 0.1;
}

float shadowRayVisibility(float3 origin, float3 direction, float minT, float maxT)
{
	// Setup our shadow ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = minT;
    ray.TMax = maxT;

	// Query if anything is between the current point and the light (i.e., at maxT) 
    ShadowRayPayload rayPayload = { 0 };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
                  RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 2, 0, ray, rayPayload);
    
	// Check if anyone was closer than our maxT distance (in which case we're occluded)
    return rayPayload.hitDist;
}

#endif