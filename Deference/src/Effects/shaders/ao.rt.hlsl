#include "Random.hlsli"

// Input and out textures that need to be set by the C++ code
Texture2D<float4> gPos : register(t0);
Texture2D<float4> gNorm : register(t1);
RWTexture2D<float4> gOutput : register(u0);

RaytracingAccelerationStructure scene : register(t2);

struct AORayPayload
{
    float aoValue; // Store 0 if we hit a surface, 1 if we miss all surfaces
};

// A constant buffer we'll populate from our C++ code 
struct RayGenCB
{
    float gAORadius;
    float gMinT;
    uint gFrameCount;
    uint gRayCount;
};

ConstantBuffer<RayGenCB> aoConstants : register(b0);

// A wrapper function that encapsulates shooting an ambient occlusion ray query
float shootAmbientOcclusionRay(float3 orig, float3 dir, float minT, float maxT)
{
	// Setup ambient occlusion ray and payload
    AORayPayload rayPayload = { 0.0f }; // Specified value is returned if we hit a surface
    RayDesc rayAO;
    rayAO.Origin = orig; // Where does our ray start?
    rayAO.Direction = dir; // What direction does our ray go?
    rayAO.TMin = minT; // Min distance to detect an intersection
    rayAO.TMax = maxT; // Max distance to detect an intersection

	// Trace our ray.  Ray stops after it's first definite hit; never execute closest hit shader
    TraceRay(scene,
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
		0xFF, 0, 1, 0, rayAO, rayPayload);

	// Copy our AO value out of the ray payload.
    return rayPayload.aoValue;
}

// How do we generate the rays that we trace?
[shader("raygeneration")]
void AoRayGen()
{
	// Where is this thread's ray on screen?
    uint2 launchIndex = DispatchRaysIndex();
    uint2 launchDim = DispatchRaysDimensions();

	// Initialize a random seed, per-pixel, based on a screen position and temporally varying count
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, aoConstants.gFrameCount);

	// Load the position and normal from our g-buffer
    float4 worldPos = gPos[launchIndex];
    float4 worldNorm = gNorm[launchIndex];

	// Default ambient occlusion value if we hit the background
    float aoVal = float(aoConstants.gRayCount);

  // worldPos.w == 0 for background pixels; only shoot AO rays elsewhere
    if (worldPos.w != 0.0f)
    {
        aoVal = 0.f;
        for (int i = 0; i < aoConstants.gRayCount; i++)
        {
            // Random ray, sampled on cosine-weighted hemisphere around normal 
            float3 worldDir = getCosHemisphereSample(randSeed, worldNorm.xyz);
             // Shoot our ambient occlusion ray and update the final AO value
            aoVal += shootAmbientOcclusionRay(worldPos.xyz, worldDir, aoConstants.gMinT, aoConstants.gAORadius);
        }
    }

    aoVal /= aoConstants.gRayCount;
    gOutput[launchIndex] = float4(aoVal, aoVal, aoVal, 1.0f);
}


// What code is executed when our ray misses all geometry?
[shader("miss")]
void AoMiss(inout AORayPayload rayData)
{
	// Our ambient occlusion value is 1 if we hit nothing.
    rayData.aoValue = RayTCurrent();

}

// What code is executed when our ray hits a potentially transparent surface?
[shader("anyhit")]
void AoAnyHit(inout AORayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
    //if (alphaTestFails(attribs))
        //IgnoreHit();
}
