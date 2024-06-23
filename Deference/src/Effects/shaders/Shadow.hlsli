#ifndef __SHADOW_HLSLI__
#define __SHADOW_HLSLI__

#include "Random.hlsli"

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

// Get a random vector in a cone centered around a specified normal direction.
float3 getConeSample(inout uint randSeed, float3 hitNorm, float cosThetaMax)
{
	// Get 2 random numbers to select our sample with
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

	// Cosine weighted hemisphere sample from RNG
    float3 bitangent = getPerpendicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);

    float cosTheta = (1.0 - randVal.x) + randVal.x * cosThetaMax;
    float r = sqrt(1.0 - cosTheta * cosTheta);
    float phi = randVal.y * 2.0 * 3.14159265f;

	// Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi)) + bitangent * (r * sin(phi)) + hitNorm.xyz * cosTheta;
}

float shadowRayVisibility(inout uint rndSeed, float3 origin, float3 direction, float minT, float maxT)
{
	// Setup our shadow ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(getConeSample(rndSeed, direction, 0.995));
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