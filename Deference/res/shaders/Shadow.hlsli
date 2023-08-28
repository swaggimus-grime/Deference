#ifndef __SHADOW_HLSLI__
#define __SHADOW_HLSLI__



struct ShadowRayPayload
{
    float hitDist;
};

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
                  RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 1, 0, ray, rayPayload);
    
	// Check if anyone was closer than our maxT distance (in which case we're occluded)
    return rayPayload.hitDist; //(rayPayload.hitDist > maxT) ? 1.0f : 0.0f;
}

#endif